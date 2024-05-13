# Build Options
#   change yes to no to disable
#
BOOTMAGIC_ENABLE = yes     # Virtual DIP switch configuration
MOUSEKEY_ENABLE = yes       # Mouse keys
EXTRAKEY_ENABLE = yes       # Audio control and System control
CONSOLE_ENABLE = no         # Console for debug
COMMAND_ENABLE = no         # Commands for debug and configuration
# if this doesn't work, see here: https://github.com/tmk/tmk_keyboard/wiki/FAQ#nkro-doesnt-work
NKRO_ENABLE = yes          # USB Nkey Rollover
BACKLIGHT_ENABLE = no       # Enable keyboard backlight functionality
RGBLIGHT_ENABLE = no       # Enable keyboard RGB underglow
ENCODER_ENABLE = no        # Enable rotary encoder support
MIDI_ENABLE = no            # MIDI support
BLUETOOTH_ENABLE = no       # Enable Bluetooth with the Adafruit EZ-Key HID
AUDIO_ENABLE = no           # Audio output
RAW_ENABLE = yes
COMBO_ENABLE = no
SPLIT_KEYBOARD = yes
SERIAL_DRIVER = vendor
DEFAULT_FOLDER = fabis/rp2040
POINTING_DEVICE_ENABLE = yes
POINTING_DEVICE_DRIVER = pmw3360
HAPTIC_ENABLE = yes
HAPTIC_DRIVER = drv2605l
OS_DETECTION_ENABLE = yes
