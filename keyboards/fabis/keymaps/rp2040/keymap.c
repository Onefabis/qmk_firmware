// Copyright 2021 Christian Eiden, cykedev
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
//
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.	 If not, see <http://www.gnu.org/licenses/>.

#include QMK_KEYBOARD_H
#include "raw_hid.h"
#include "drivers/haptic/drv2605l.h"
#include "print.h"

bool extra_sensor_button_left = false;
bool extra_sensor_button_right = false;
bool cursor_button_left = false;
bool cursor_button_right = false;
bool haptic_master = false;
bool haptic_slave = false;

int16_t delta_left_x = 0;
int16_t delta_left_y = 0;
int16_t delta_right_x = 0;
int16_t delta_right_y = 0;
static uint16_t haptic_timer = 0;

int16_t os_id = 0;

const uint16_t cpi[4] = {600, 5400, 6600, 7500};
#define DPI_OPTION_SIZE (sizeof(cpi) / sizeof(uint16_t))
int cpi_sel = 2;

#define CUSTOM_SAFE_RANGE SAFE_RANGE
#define LANG_CHANGE_DEFAULT LANG_CHANGE_ALT_SHIFT

#include "arbitrary_keycode/include.h"
#include "lang_shift/include.h"

enum custom_keycodes {
	KEYCODES_START = CUSTOM_SAFE_RANGE,
	CT_C,
	CT_X,
	CT_V,
	CT_A,
	C_LN,
	CT_ENT,
	CT_BSPC,
	CT_DEL,
	CT_Z,
	CT_Y,
	CT_PLUS,
	CT_MINUS,
	CT_F,
	CT_LT,
	CT_RT,
	CT_UP,
	CT_DN,
	LT_GT,
	EQ_EQ,
	GT_EQ,
	LT_EQ,
	DB_BR,
	DB_RN,
	DB_RN_EN,
	DB_SB,
	LG_SW,
	KC_FSPC,
	LED_PLUS,
	LED_MINUS,
	LED_TOGGLE,
	SCROLL,
	MOUSE_OFF,
	SPACE,
	DB_AG,
	EX_BT_L,
	EX_BT_R,
	DPI_CONFIG,
	CURSOR_L,
	CURSOR_R
};

#    ifdef TAPPING_TERM_PER_KEY
#        define TAP_CHECK get_tapping_term(KC_BTN1, NULL)
#    else
#        ifndef TAPPING_TERM
#            define TAPPING_TERM 175
#        endif
#        define TAP_CHECK TAPPING_TERM
#    endif

bool            tap_toggling          = false;
int16_t delta_x = 0;
int16_t delta_y = 0;
int16_t scroll_timer = 0;
bool force_trackball_arrows = false;
uint8_t qwerty_toggle = 0;
#ifdef RAW_ENABLE
#define RAW_EPSIZE 32
uint8_t response[RAW_EPSIZE];
#endif
uint8_t thisHand;
uint8_t thatHand;

int basicLayerNum = 0;
// Defines names for use in layer keycodes and the keymap
enum layer_names {
	_EN = 0,
	_EN_SH = 1,
	_RU = 2,
	_RU_SH= 3,
	_EXTRAZ = 4,
	_ALT = 5,
};

#define LGUI_0 LGUI(KC_0)
#define LGUI_1 LGUI(KC_1)
#define LGUI_2 LGUI(KC_2)
#define LGUI_3 LGUI(KC_3)
#define LGUI_4 LGUI(KC_4)
#define LGUI_5 LGUI(KC_5)
#define LGUI_6 LGUI(KC_6)
#define LGUI_7 LGUI(KC_7)
#define LGUI_8 LGUI(KC_8)
#define LGUI_9 LGUI(KC_9)
#define CT_A_0 LCA(KC_0)
#define CT_A_1 LCA(KC_1)
#define CT_A_2 LCA(KC_2)
#define CT_A_3 LCA(KC_3)
#define CT_A_4 LCA(KC_4)
#define CT_A_5 LCA(KC_5)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[_ALT] = LAYOUT_5x6_5(
    XXXXXXX,       KC_5,         KC_4,         KC_3,         KC_2,         KC_1,  			       KC_0,          KC_9,          KC_8,          KC_7,          KC_6,          KC_INS,
	XXXXXXX,       XXXXXXX,      LGUI_7,       LGUI_8,       LGUI_9,       LCAG(KC_C), 			   EN_CIRC,       KC_MUTE,       KC_VOLD,       KC_VOLU,       XXXXXXX,       XXXXXXX,
	XXXXXXX,       XXXXXXX,      LGUI_4,       LGUI_5,       LGUI_6,       XXXXXXX,  			   KC_CALC,       XXXXXXX,       XXXXXXX,       XXXXXXX,       XXXXXXX,       XXXXXXX,
	KC_NUM,        XXXXXXX,      LGUI_1,       LGUI_2,       LGUI_3,       LGUI_0,				   C_LN,          CT_C,          CT_V,       	CT_X,          CT_A,       DPI_CONFIG,

                                 XXXXXXX,      XXXXXXX,      CTRL_0,       OSM(MOD_LSFT),          XXXXXXX,       XXXXXXX,       CT_X,          CT_V,
	                                                         C(KC_F),      ALT_0,                  AG_SDOT,       CT_C,
	EX_BT_L,       CURSOR_L,	 KC_MS_BTN2,   KC_MS_BTN4,   KC_MS_BTN1,   KC_LGUI,                LA_SYNC,       KC_MS_BTN1,    KC_MS_BTN4,    KC_MS_BTN2,    CURSOR_R,      EX_BT_R
	),

	[_EXTRAZ] = LAYOUT_5x6_5(
    KC_F1,         KC_F2,        KC_F3,        KC_F4,        KC_F5,        KC_F6,  			       KC_F7,         KC_F8,         KC_F9,         KC_F10,        KC_F11,        TG(_EXTRAZ),
	LCAG(KC_E),    KC_F12,       KC_P7,        KC_P8,        KC_P9,        S(KC_TAB),              AG_ASTR,       EN_LPRN,       EN_RPRN,       AG_PERC,       KC_AMPR,       AG_BSLS,
	LCAG(KC_R),    EN_TILD,      KC_P4,        KC_P5,        KC_P6,        KC_TAB,                 LCAG(KC_D),    EN_LBRC,       EN_RBRC,       EN_LT,         EN_GT,         LCAG(KC_A),
	EN_DLR,        LCAG(KC_P),   KC_P1,        KC_P2,        KC_P3,        KC_P0,	    	       AG_UNDS,       EN_LCBR,       EN_RCBR,       EN_PIPE,       EN_GRV,        EN_HASH,

                                 XXXXXXX,      XXXXXXX,      CTRL_0,  	   OSM(MOD_LSFT),          XXXXXXX,       XXXXXXX,       CT_X,          CT_V,
	                                                         C(KC_F),      ALT_0,                  AG_SDOT,        CT_C,
	EX_BT_L,       CURSOR_L,	 KC_MS_BTN2,   KC_MS_BTN4,   KC_MS_BTN1,   KC_LGUI,                LA_CHNG,       KC_MS_BTN1,    KC_MS_BTN4,    KC_MS_BTN2,    CURSOR_R,      EX_BT_R
	),

	[_EN] = LAYOUT_5x6_5(
	KC_ESC,        AG_MINS,      AG_SCLN,      AG_EQL,       AG_COLN,      AG_EXCL,				   AG_QUES,       EN_QUOT,       AG_CMSP,       AG_SLSH,       AG_PLUS,       TG(_EXTRAZ),
	EN_AT,         XXXXXXX,      EN_Y,         EN_O,         EN_U,         XXXXXXX, 			   XXXXXXX,       EN_L,          EN_D,          EN_W,          EN_B,          EN_Z,
	AG_DQUO,       EN_C,         EN_I,         EN_E,         EN_A,         XXXXXXX,  			   EN_V,          EN_H,          EN_T,          EN_S,          EN_N,          EN_Q,
	C(KC_Z),       EN_G,         EN_X,         EN_J,         EN_K,         XXXXXXX,				   XXXXXXX,       EN_R,          EN_M,          EN_F,          EN_P,          XXXXXXX,

                                 XXXXXXX,      XXXXXXX,      CTRL_0,	   SFT_N, 	               XXXXXXX,       XXXXXXX,       KC_DEL,        KC_BSPC,
	                                                         KC_ENT,       ALT_0,                  AG_DOT,        KC_SPC,
	EX_BT_L,       CURSOR_L,	 KC_MS_BTN2,   KC_MS_BTN4,   KC_MS_BTN1,   OSL(4),                 LA_CHNG,       KC_MS_BTN1,    KC_MS_BTN4,    KC_MS_BTN2,    CURSOR_R,      EX_BT_R
	),

	[_EN_SH] = LAYOUT_5x6_5(
	KC_ESC,        KC_5,         KC_4,         KC_3,         KC_2,         KC_1, 				   KC_0,          KC_9,          KC_8,          KC_7,          KC_6,          TG(_EXTRAZ),
	EN_UNDS,       XXXXXXX,      EN_S_Y,       EN_S_O,       EN_S_U,       XXXXXXX,				   XXXXXXX,       EN_S_L,        EN_S_D,        EN_S_W,        EN_S_B,        EN_S_Z,
	EN_HASH,       EN_S_C,       EN_S_I,       EN_S_E,       EN_S_A,       XXXXXXX,		  		   EN_S_V,        EN_S_H,        EN_S_T,        EN_S_S,        EN_S_N,        EN_S_Q,
	C(KC_Y),       EN_S_G,       EN_S_X,       EN_S_J,       EN_S_K,       XXXXXXX,	               XXXXXXX,       EN_S_R,        EN_S_M,        EN_S_F,        EN_S_P,        XXXXXXX,

                                 XXXXXXX,      XXXXXXX,      CTRL_0,  	   MOD_LSFT,               XXXXXXX,       XXXXXXX,       KC_DEL,        KC_BSPC,
	                                                         KC_ENT,       ALT_0,                  AG_DOT,        KC_SPC,
	EX_BT_L,       CURSOR_L,	 KC_MS_BTN2,   KC_MS_BTN4,   KC_MS_BTN1,   OSL(4),                 LA_CHNG,       KC_MS_BTN1,    KC_MS_BTN4,    KC_MS_BTN2,    CURSOR_R,      EX_BT_R
	),

	[_RU] = LAYOUT_5x6_5(
	KC_ESC,        AG_MINS,      AG_SCLN,      AG_EQL,       AG_COLN,      AG_EXCL,				   AG_QUES,       EN_QUOT,       AG_CMSP,       AG_SLSH,       AG_PLUS,       TG(_EXTRAZ),
	EN_AT,         RU_F,         RU_SF,        RU_H,         RU_JA,        RU_Y,                   RU_Z,          RU_V,          RU_K,          RU_D,          RU_CH,         RU_SH,
	AG_DQUO,       RU_U,         RU_I,         RU_JE,        RU_O,         RU_A,                   RU_L,          RU_N,          RU_T,          RU_S,          RU_R,          RU_J,
	C(KC_Z),       RU_JO,        RU_HD,        RU_E,         RU_JU,        RU_TS,                  RU_B,          RU_M,          RU_P,          RU_G,          RU_ZH,         RU_SC,

                                 XXXXXXX,      XXXXXXX,      CTRL_0,	   OSM(MOD_LSFT), 	       XXXXXXX,       XXXXXXX,       KC_DEL,        KC_BSPC,
	                                                         KC_ENT,       ALT_0,                  AG_DOT,        KC_SPC,
	EX_BT_L,       CURSOR_L,	 KC_MS_BTN2,   KC_MS_BTN4,   KC_MS_BTN1,   OSL(4),                 LA_CHNG,       KC_MS_BTN1,    KC_MS_BTN4,    KC_MS_BTN2,    CURSOR_R,      EX_BT_R
	),

	[_RU_SH] = LAYOUT_5x6_5(
	KC_ESC,        AG_5,         AG_4,         AG_3,         AG_2,         AG_1,                   AG_0,	      AG_9,          AG_8,          AG_7,          AG_6,          TG(_EXTRAZ),
	EN_UNDS,       RU_S_F,       RU_S_SF,      RU_S_H,       RU_S_JA,      RU_S_Y,                 RU_S_Z,        RU_S_V,        RU_S_K,        RU_S_D,        RU_S_CH,       RU_S_SH,
	EN_HASH,       RU_S_U,       RU_S_I,       RU_S_JE,      RU_S_O,       RU_S_A,                 RU_S_L,        RU_S_N,        RU_S_T,        RU_S_S,        RU_S_R,        RU_S_J,
	C(KC_Y),       RU_S_JO,      RU_S_HD,      RU_S_E,       RU_S_JU,      RU_S_TS,                RU_S_B,        RU_S_M,        RU_S_P,        RU_S_G,        RU_S_ZH,       RU_S_SC,

                                 XXXXXXX,      XXXXXXX,      CTRL_0,  	   MOD_LSFT,               XXXXXXX,       XXXXXXX,       KC_DEL,        KC_BSPC,
	                                                         KC_ENT,       ALT_0,                  AG_DOT,        KC_SPC,
	EX_BT_L,       CURSOR_L,	 KC_MS_BTN2,   KC_MS_BTN4,   KC_MS_BTN1,   OSL(4),                 LA_CHNG,       KC_MS_BTN1,    KC_MS_BTN4,    KC_MS_BTN2,    CURSOR_R,      EX_BT_R
	),


};

void persistent_default_layer_set(uint16_t default_layer) {
	default_layer_set(default_layer);
}

void pointing_device_set_new_cpi(uint16_t cpi_value) {
    pointing_device_set_cpi(cpi_value);
    pointing_device_set_cpi_on_side(false, cpi_value+950);
}

void pointing_device_change_cpi(void) {
    cpi_sel+=1;
    if(cpi_sel>=DPI_OPTION_SIZE){
    	cpi_sel=0;
    }
	pointing_device_set_new_cpi(cpi[cpi_sel]);

    #ifdef RAW_ENABLE
        uint8_t response_raw[RAW_EPSIZE];
        memset(response_raw, 0, RAW_EPSIZE);
        response_raw[0] = 255;
        response_raw[1] = cpi_sel;
        if(os_id==0){
            raw_hid_send(response_raw, RAW_EPSIZE);
        }
    #endif
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
	if (!lang_shift_process_record(keycode, record)) return false;
	switch (keycode) {
		case CT_Z:
			if (record->event.pressed) {
				lang_activate(0);
				register_code16(C(KC_Z));
				unregister_code16(C(KC_Z));
			}
			return false;
			break;
		case CT_C:
			if (record->event.pressed) {
				lang_activate(0);
				register_code16(C(KC_C));
				unregister_code16(C(KC_C));
			}
			return false;
			break;
		case CT_X:
			if (record->event.pressed) {
				lang_activate(0);
				register_code16(C(KC_X));
				unregister_code16(C(KC_X));
			}
			return false;
			break;
		case CT_V:
			if (record->event.pressed) {
				lang_activate(0);
				register_code16(C(KC_V));
				unregister_code16(C(KC_V));
			}
			return false;
			break;
		case CT_A:
			if (record->event.pressed) {
				lang_activate(0);
				register_code16(C(KC_A));
				unregister_code16(C(KC_A));
			}
			return false;
			break;
		case C_LN:
			if (record->event.pressed) {
				lang_activate(0);
				register_code16(KC_BTN1);
				unregister_code16(KC_BTN1);
				register_code16(KC_BTN1);
				unregister_code16(KC_BTN1);
				register_code16(KC_BTN1);
				unregister_code16(KC_BTN1);
				register_code16(C(KC_C));
				unregister_code16(C(KC_C));
				register_code16(KC_BTN1);
				unregister_code16(KC_BTN1);
			}
			return false;
			break;
		case CURSOR_L:
			if (record->event.pressed) {
				cursor_button_left = true;
				delta_left_x = 0;
				delta_left_y = 0;
				delta_right_x = 0;
				delta_right_y = 0;
			} else {
				cursor_button_left = false;
			}
			return false;
			break;
		case CURSOR_R:
			if (record->event.pressed) {
				cursor_button_right = true;
				delta_left_x = 0;
				delta_left_y = 0;
				delta_right_x = 0;
				delta_right_y = 0;
			} else {
				cursor_button_right = false;
			}
			return false;
			break;
		case EX_BT_L:
			if (record->event.pressed) {
				layer_on(_ALT);
				extra_sensor_button_left = true;
				delta_left_x = 0;
				delta_left_y = 0;
				delta_right_x = 0;
				delta_right_y = 0;
			} else {
				if (extra_sensor_button_right != true){
					layer_off(_ALT);
				}
				extra_sensor_button_left = false;
			}
			return false;
			break;
		case EX_BT_R:
			if (record->event.pressed) {
			    layer_on(_ALT);
				extra_sensor_button_right = true;
				delta_left_x = 0;
				delta_left_y = 0;
				delta_right_x = 0;
				delta_right_y = 0;
			} else {
				if (extra_sensor_button_left != true){
					layer_off(_ALT);
				}
				extra_sensor_button_right = false;
			}
			return false;
			break;
		default:
			return true;
	};
};


void user_timer(void) {
	lang_shift_user_timer();
};


void matrix_scan_user(void) {
	user_timer();
};

void housekeeping_task_user(void) {
	extern uint8_t split_haptic_play;
	if (haptic_master == true){
		drv2605l_pulse(DRV2605L_EFFECT_SHARP_CLICK_30);
		split_haptic_play = DRV2605L_EFFECT_SHARP_CLICK_30;
		haptic_master = false;
	};
	if (haptic_slave == true){
		drv2605l_pulse(DRV2605L_EFFECT_SHARP_CLICK_30);
		split_haptic_play = DRV2605L_EFFECT_SHARP_CLICK_30;
		haptic_slave = false;
	}
};

#ifdef OS_DETECTION_ENABLE
bool process_detected_host_os_kb(os_variant_t detected_os) {
    if (!process_detected_host_os_user(detected_os)) {
        return false;
    }
    switch (detected_os) {
        case OS_MACOS:
        case OS_IOS:
            os_id = 1;
            break;
        case OS_WINDOWS:
            os_id = 0;
            break;
        case OS_LINUX:
            os_id = 2;
            break;
        case OS_UNSURE:
            os_id = 3;
            break;
    }

    return true;
}
#endif

#ifdef RAW_ENABLE
layer_state_t layer_state_set_user(layer_state_t state) {
	uint8_t response_raw[RAW_EPSIZE];
    memset(response_raw, 0, RAW_EPSIZE);
	switch (get_highest_layer(state)) {
	case _EN:
		response_raw[0] = 0;
		break;
	case _RU:
		response_raw[0] = 1;
		break;
	case _EXTRAZ:
		response_raw[0] = 2;
		break;
	case _ALT:
		response_raw[0] = 3;
		break;
	default:
		response_raw[0] = 0;
		break;
	};
	response_raw[1] = cpi_sel;
    if(os_id==0){
        raw_hid_send(response_raw, RAW_EPSIZE);
    }
	return state;
}

void raw_hid_receive(uint8_t *data, uint8_t length) {
	if(data[0] == 0) {
		lang_toggle(1);
	} else if(data[0] == 1) {
		lang_toggle(0);
	};
};
#endif

bool process_record_kb(uint16_t keycode, keyrecord_t* record) {
	if (!process_record_user(keycode, record)) {
		return false;
	}
	#ifdef POINTING_DEVICE_ENABLE
		if (keycode == DPI_CONFIG && record->event.pressed) {
	    pointing_device_change_cpi();
	    return false;
	}
	#endif
    return true;
};

#ifdef POINTING_DEVICE_ENABLE

void keyboard_post_init_user(void) {
	pointing_device_set_new_cpi(cpi[cpi_sel]);
}

report_mouse_t pointing_device_task_combined_user(report_mouse_t left_report, report_mouse_t right_report) {
	if (layer_state_is(_ALT) || (cursor_button_left == true || cursor_button_right == true) ){
		if (haptic_timer ==0){
			haptic_timer = timer_read();
		}
		if (extra_sensor_button_right == true || cursor_button_left == true){
		    delta_left_y += left_report.y;
		    if (delta_left_y > 120) {
                left_report.v = -1;
                delta_left_y = 0;
            } else if (delta_left_y < -120) {
          	    left_report.v = 1;
                delta_left_y = 0;
            };
            left_report.x = left_report.y = 0;

			delta_right_x += right_report.x;
			if (delta_right_x > 450) {
				tap_code(KC_RIGHT);
				if (timer_elapsed(haptic_timer) > 29){
					if (is_keyboard_master()){
						haptic_master = true;
					} else {
						haptic_slave = true;
					}
					haptic_timer = 0;
				};
				delta_right_x = 0;
			} else if (delta_right_x < -450) {
				tap_code(KC_LEFT);
				if (timer_elapsed(haptic_timer) > 29){
					if (is_keyboard_master()){
						haptic_master = true;
					} else {
						haptic_slave = true;
					}
					haptic_timer = 0;
				};
				delta_right_x = 0;
			};
            right_report.x = right_report.y = 0;
		} else if (extra_sensor_button_left == true || cursor_button_right == true){
		    delta_right_y += right_report.y;
		    if (delta_right_y > 120) {
                right_report.v = -1;
                delta_right_y = 0;
            } else if (delta_right_y < -120) {
          	    right_report.v = 1;
                delta_right_y = 0;
            };
            right_report.x = right_report.y = 0;

			delta_left_y += left_report.y;
			if (delta_left_y > 450) {
				tap_code(KC_DOWN);
				if (timer_elapsed(haptic_timer) > 29){
					if (is_keyboard_master()){
						haptic_slave = true;
					} else {
						haptic_master = true;
					}
					haptic_timer = 0;
				};
				delta_left_y = 0;
			} else if (delta_left_y < -450) {
				tap_code(KC_UP);
				if (timer_elapsed(haptic_timer) > 29){
					if (is_keyboard_master()){
						haptic_slave = true;
					} else {
						haptic_master = true;
					}
					haptic_timer = 0;
				};
				delta_left_y = 0;
			};
            left_report.x = left_report.y = 0;
		};
	};
	return pointing_device_combine_reports(left_report, right_report);
};
#endif

