/*****************************************************************************
*  Copyright (c) 2012 Duane Ryan Bailey                                      *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*****************************************************************************/

#include <Render/TokenRender.h>

#include <cstdio>
#include <iostream>

#include <Render/LineBreaker.h>

using namespace tex;

void TokenRender::init_from_file(const char *path, const Codec *codec, UniquePtr<TokenRender> &result) {
  UniquePtr<TokenRender> render;
  render.reset(new TokenRender());
  TokenInputStream::init_from_file(path, codec, render->input);
  result.reset(render.take());
}

#define M(mode, cmd) ((mode) << 16 | (cmd))

static inline void end_paragraph(UniquePtr<State> &state) {
  line_break(state);
  state->render().set_mode(VMODE);
}

static inline void begin_paragraph(UniquePtr<State> &state, bool indent) {
  RenderState &render = state->render();
  if (render.mode() == VMODE) {
    render.push();
    render.set_mode(HMODE);
    render.set_head(NULL);
    render.set_tail(NULL);
  }
  if (indent) {
    glue_node glue = skip_glue(state->eqtb()[PARINDENT_CODE].scaled);
    RenderNode *indent_glue = RenderNode::new_glue(glue);
    render.append(indent_glue);
  }
}

void TokenRender::render_input(UniquePtr<State> &state) {
  Token token;
  RenderState &render = state->render();
  bool stop = false;
  while (!stop) {
    input->peek_token(state, token);
    uint32_t mode_cmd = (render.mode() << 16 | (token.cmd & 0xFFFF));
    switch(mode_cmd) {
      case M(VMODE, CC_SPACER): {
        input->consume_token(state, token);
        break;
      }
      case M(VMODE, CC_LETTER):
      case M(VMODE, CC_OTHER_CHAR): {
        // enter horizontal mode.
        begin_paragraph(state, true);
        break; // read the character again, this time in HMODE
      }
      case M(HMODE, CC_LETTER):
      case M(HMODE, CC_OTHER_CHAR): {
        MutableUString mut_string;
        // first, read characters into the char array.
        while (token.cmd == CC_LETTER || token.cmd == CC_OTHER_CHAR) {
          input->consume_token(state, token);
          mut_string.add(token.uc);
          input->peek_token(state, token);
        }
        // now we've hit a new token. token is invalidated, but we can still
        // process valid characters we've already read in. The font has not
        // changed, so we can append char/kerning/ligature nodes as normal.
        uint32_t font = state->eqtb()[FONT_CODE].u64;
        std::list<set_op> *op_list =
          state->metrics(font).set_string(mut_string);
        for (std::list<set_op>::iterator iter = op_list->begin();
                                         iter != op_list->end();
                                         iter++) {
          set_op op = *iter;
          if (op.type == OP_SET)
            render.append(RenderNode::char_rnode(op.code, font));
          else if (op.type == OP_SET_LIG) {
            RenderNode *inner_head = NULL, *inner_tail = NULL;
            for (unsigned i = 0; i < op.lig.extent; i++) {
              RenderNode *inner_char = RenderNode::char_rnode(
                                       mut_string[op.lig.idx + i], font);
              if (inner_head == NULL)
                inner_head = inner_tail = inner_char;
              else {
                inner_tail->link = inner_char;
                inner_tail = inner_char;
                inner_tail->link = NULL;
              }
            }
            render.append(RenderNode::new_lig(op.lig.code, font, inner_head));
          }
          else
            render.append(RenderNode::new_kern(KERN_NORMAL, op.kern));
        }
        delete op_list;
        break;
      }
      case M(HMODE, CC_SPACER): {
        input->consume_token(state, token);
        Font &font = state->metrics(state->eqtb()[FONT_CODE].u64);
        RenderNode *node = RenderNode::glue_rnode(
          font.space(), font.space_stretch(), font.space_shrink(),
          GLUE_NORMAL, GLUE_NORMAL);
        render.append(node);
        break;
      }
      case M(HMODE, CC_PAR_END): {
        input->consume_token(state, token);
        end_paragraph(state);
        state->builder().build_page(state);
        break;
      }
      case M(VMODE, CC_PAR_END): {
        input->consume_token(state, token);
        break;
      }
      case M(HMODE, CC_STOP): {
        input->consume_token(state, token);
        // leave HMODE
        end_paragraph(state);
        state->builder().build_page(state);
        state->builder().ship_page(state);
        stop = true;
        break;
      }
      case M(VMODE, CC_STOP): {
        input->consume_token(state, token);
        state->builder().ship_page(state);
        stop = true;
        break;
      }
      case M(VMODE, SET_FONT_CODE):
      case M(HMODE, SET_FONT_CODE):
      case M(MMODE, SET_FONT_CODE):
      case M(IN_VMODE, SET_FONT_CODE):
      case M(IN_HMODE, SET_FONT_CODE):
      case M(IN_MMODE, SET_FONT_CODE): {
        input->consume_token(state, token);
        state->eqtb()[FONT_CODE].u64 = token.cs->operand.i64;
        break;
      }
      case M(VMODE, CC_LBRACE):
      case M(HMODE, CC_LBRACE):
      case M(MMODE, CC_LBRACE):
      case M(IN_VMODE, CC_LBRACE):
      case M(IN_HMODE, CC_LBRACE):
      case M(IN_MMODE, CC_LBRACE): {
        input->consume_token(state, token);
        state->eqtb().enter_grouping();
        break;
      }
      case M(HMODE, EJECT_CODE):
        end_paragraph(state);
      case M(VMODE, EJECT_CODE): {
        input->consume_token(state, token);
        render.append(RenderNode::new_penalty(PENALTY_BREAK));
        state->builder().build_page(state);
        break;
      }
      case M(VMODE, PAR_BEG_CODE):
      case M(HMODE, PAR_BEG_CODE): {
        input->consume_token(state, token);
        begin_paragraph(state, token.cs->operand.i64 == 0 ? false : true);
        break;
      }
      case M(HMODE, RULE_CODE): {
        end_paragraph(state);
      }
      case M(VMODE, RULE_CODE): {
        input->consume_token(state, token);
        RenderNode *rule = RenderNode::new_rule(
                            state->eqtb()[HSIZE_CODE].scaled,
                            scaled(1 << 16));
        glue_node neg_baseline_glue
          = skip_glue(-state->eqtb()[BASELINE_SKIP_CODE].scaled / 2);
        glue_node baseline_glue
          = skip_glue(state->eqtb()[BASELINE_SKIP_CODE].scaled / 2);
        render.append(RenderNode::new_glue(neg_baseline_glue));
        render.append(rule);
        render.append(RenderNode::new_glue(baseline_glue));
        state->builder().build_page(state);
        break;
      }
      case M(VMODE, SKIP_CODE): {
        begin_paragraph(state, false);
        break;
      }
      case M(HMODE, SKIP_CODE): {
        input->consume_token(state, token);
        RenderNode *skip_node
        = RenderNode::new_glue(*(glue_node*)(token.cs->operand.ptr));
        render.append(skip_node);
        break;
      }
      case M(HMODE, PENALTY_CODE): {
        input->consume_token(state, token);
        render.append(RenderNode::new_penalty(token.cs->operand.i64));
        break;
      }
      case M(VMODE, CC_RBRACE):
      case M(HMODE, CC_RBRACE):
      case M(MMODE, CC_RBRACE):
      case M(IN_VMODE, CC_RBRACE):
      case M(IN_HMODE, CC_RBRACE):
      case M(IN_MMODE, CC_RBRACE): {
        input->consume_token(state, token);
        tex_eqtb &eqtb = state->eqtb();
        if (eqtb.level() <= 1)
          throw new GenericDiag("No matching brace for right bracket",
                                DIAG_PARSE_ERR, BLAME_HERE);
        eqtb.leave_grouping();
        break;
      }
      default: {
        std::cout << token.cmd;
        throw new BlameSourceDiag("Command code not implemented yet.",
          DIAG_RENDER_ERR, BLAME_HERE,
          BlameSource("file", token.line, token.line, token.col, token.col));
      }
    }
  }
}