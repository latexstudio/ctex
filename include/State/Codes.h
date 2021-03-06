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

#ifndef __INCLUDE_STATE_CODES_H__
#define __INCLUDE_STATE_CODES_H__

namespace tex {

enum {
  CC_ESCAPE = 0,
  CC_RELAX = 0,
  CC_LBRACE = 1,
  CC_RBRACE = 2,
  CC_MATH_SHIFT = 3,
  CC_TAB_MARK = 4,
  CC_CAR_RET = 5,
  CC_OUT_PARAM = 5,
  CC_MAC_PARAM = 6,
  CC_SUP_MARK = 7,
  CC_SUB_MARK = 8,
  CC_IGNORE = 9,
  CC_ENV = 9,
  CC_SPACER = 10,
  CC_LETTER = 11,
  CC_OTHER_CHAR = 12,
  CC_ACTIVE_CHAR = 13,
  CC_PAR_END = 13,
  CC_MATCH = 13,
  CC_COMMENT = 14,
  CC_END_MATCH = 14,
  CC_STOP = 14,
  CC_INVALID = 15,
  CC_DELIM_NUM = 15,
  
  SET_FONT_CODE, // 16
  EJECT_CODE, // 17
  PAR_BEG_CODE, // 18
  PENALTY_CODE, // 19
  SKIP_CODE, // 20
  RULE_CODE, // 21
};

enum {
  PRETOLERANCE_CODE,
  LEFT_SKIP_CODE,
  RIGHT_SKIP_CODE,
  PARINDENT_CODE,
  BASELINE_SKIP_CODE,
  SPLIT_TOP_SKIP_CODE,
  SPLIT_MAX_DEPTH_CODE,
  VSIZE_CODE,
  HSIZE_CODE,
  FONT_CODE,
  ADJ_DEMERITS_CODE,
  LINE_PENALTY_CODE,
  BOX_BEGIN_CODE,
  MAX_INTERNAL_CODE = BOX_BEGIN_CODE+256,
};

}

#endif  // _INCLUDE_STATE_CODES_H__