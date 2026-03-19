#pragma warning disable IDE1006 // Naming Styles
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

#pragma warning disable CS1591 // Missing XML comment for publicly visible type or member

namespace Script.Library
{
    /// <summary>
    /// Keyboard and mouse query API for scripts.
    /// </summary>
    public static class Input
    {
        /// <summary>
        /// Initialize internal key/mouse click tracking state.
        /// </summary>
        static Input()
        {
            foreach (KeyCode key in Enum.GetValues(typeof(KeyCode)))
            {
                KeyClicked[key] = false;
            }
            foreach (Mouse mouseBtn in Enum.GetValues(typeof(Mouse)))
            {
                mouseBtnClicked[mouseBtn] = false;
            }
        }

        /// <summary>
        /// Gets the current mouse position in window coordinates.
        /// </summary>
        public static void getCurMosusePos(out Vector2D mousePos)
        {
            PrivateAPI.InternalCall.getMousePostion(out mousePos);
        }

        /// <summary>
        /// Gets the current mouse position in window coordinates.
        /// </summary>
        public static void getMouseDelta(out Vector2D mouseDelta)
        {
            PrivateAPI.InternalCall.getMouseDelta(out mouseDelta);
        }

        /// <summary>
        /// Returns true while the given mouse button is being held down.
        /// </summary>
        /// <usage>
        //bool isPressed = Input.isMousePressed(Mouse.kMOUSE_BUTTON_LEFT);
        /// </usage>
        public static bool isMousePressed(Mouse btn)
        {
            return PrivateAPI.InternalCall.isMousePressed((int)btn);
        }

        /// <summary>
        /// Returns true on the frame the given mouse button is released.
        /// </summary>
        public static bool isMouseReleased(Mouse btn)
        {
            return PrivateAPI.InternalCall.isMouseReleased((int)btn);
        }

        /// <summary>
        /// Returns true once when the given mouse button is clicked (press without auto-repeat).
        /// </summary>
        public static bool isMouseClicked(Mouse btn)
        {
            if (isMousePressed(btn) && !mouseBtnClicked[btn])
            {
                mouseBtnClicked[btn] = true;
                return true;
            }
            if (isMouseReleased(btn) && mouseBtnClicked[btn])
            {
                mouseBtnClicked[btn] = false;
            }
            return false;
        }

        /// <summary>
        /// Gets the scroll-wheel delta since last frame. usually only the y-value of the offset is useful
        /// </summary>
        public static void mouseScroll(out Vector2D offset)
        {
            PrivateAPI.InternalCall.mouseScroll(out offset);
        }

        /// <summary>
        /// Returns true while the given key is held down.
        /// </summary>
        public static bool isKeyPressed(KeyCode key)
        {
            return PrivateAPI.InternalCall.isKeyPressed((int)key);
        }
        /// <summary>
        /// Returns true on the frame the given key is released.
        /// </summary>
        public static bool isKeyReleased(KeyCode key)
        {
            return PrivateAPI.InternalCall.isKeyReleased((int)key);
        }
        /// <summary>
        /// Returns true once when the given key is clicked (press without auto-repeat).
        /// </summary>
        public static bool isKeyClicked(KeyCode key)
        {
            if (isKeyPressed(key) && !KeyClicked[key])
            {
                KeyClicked[key] = true;
                return true;
            }
            if (isKeyReleased(key) && KeyClicked[key])
            {
                KeyClicked[key] = false;
            }
            return false;
        }

        public static bool isMouseEnable()
        {
            return PrivateAPI.InternalCall.getMouseState();
        }
        public static void enableMouse()
        {
            PrivateAPI.InternalCall.setMouseState(true);
        }
        public static void disableMouse()
        {
            PrivateAPI.InternalCall.setMouseState(false);
        }

#pragma warning disable 1591
        public enum KeyCode
        {
            kSPACE = 32,
            kAPOSTROPHE = 39,  /* ' */
            kCOMMA = 44,  /* , */
            kMINUS = 45,  /* - */
            kPERIOD = 46,  /* . */
            kSLASH = 47,  /* / */
            k0 = 48,
            k1 = 49,
            k2 = 50,
            k3 = 51,
            k4 = 52,
            k5 = 53,
            k6 = 54,
            k7 = 55,
            k8 = 56,
            k9 = 57,
            kSEMICOLON = 59,  /* ; */
            kEQUAL = 61,  /* = */
            kA = 65,
            kB = 66,
            kC = 67,
            kD = 68,
            kE = 69,
            kF = 70,
            kG = 71,
            kH = 72,
            kI = 73,
            kJ = 74,
            kK = 75,
            kL = 76,
            kM = 77,
            kN = 78,
            kO = 79,
            kP = 80,
            kQ = 81,
            kR = 82,
            kS = 83,
            kT = 84,
            kU = 85,
            kV = 86,
            kW = 87,
            kX = 88,
            kY = 89,
            kZ = 90,
            kLEFT_BRACKET = 91,  /* [ */
            kBACKSLASH = 92,  /* \ */
            kRIGHT_BRACKET = 93,  /* ] */
            kGRAVE_ACCENT = 96, /* ` */
            kWORLD_1 = 161, /* non-US #1 */
            kWORLD_2 = 162, /* non-US #2 */

            /* Function keys */
            kESCAPE = 256,
            kENTER = 257,
            kTAB = 258,
            kBACKSPACE = 259,
            kINSERT = 260,
            kDELETE = 261,
            kRIGHT = 262,
            kLEFT = 263,
            kDOWN = 264,
            kUP = 265,
            kPAGE_UP = 266,
            kPAGE_DOWN = 267,
            kHOME = 268,
            kEND = 269,
            kCAPS_LOCK = 280,
            kSCROLL_LOCK = 281,
            kNUM_LOCK = 282,
            kPRINT_SCREEN = 283,
            kPAUSE = 284,
            kF1 = 290,
            kF2 = 291,
            kF3 = 292,
            kF4 = 293,
            kF5 = 294,
            kF6 = 295,
            kF7 = 296,
            kF8 = 297,
            kF9 = 298,
            kF10 = 299,
            kF11 = 300,
            kF12 = 301,
            kF13 = 302,
            kF14 = 303,
            kF15 = 304,
            kF16 = 305,
            kF17 = 306,
            kF18 = 307,
            kF19 = 308,
            kF20 = 309,
            kF21 = 310,
            kF22 = 311,
            kF23 = 312,
            kF24 = 313,
            kF25 = 314,
            kKP_0 = 320,
            kKP_1 = 321,
            kKP_2 = 322,
            kKP_3 = 323,
            kKP_4 = 324,
            kKP_5 = 325,
            kKP_6 = 326,
            kKP_7 = 327,
            kKP_8 = 328,
            kKP_9 = 329,
            kKP_DECIMAL = 330,
            kKP_DIVIDE = 331,
            kKP_MULTIPLY = 332,
            kKP_SUBTRACT = 333,
            kKP_ADD = 334,
            kKP_ENTER = 335,
            kKP_EQUAL = 336,
            kLEFT_SHIFT = 340,
            kLEFT_CONTROL = 341,
            kLEFT_ALT = 342,
            kLEFT_SUPER = 343,
            kRIGHT_SHIFT = 344,
            kRIGHT_CONTROL = 345,
            kRIGHT_ALT = 346,
            kRIGHT_SUPER = 347,
            kMENU = 348,
        }
#pragma warning restore 1591
        /*
            enum class that represent the mouse button
        */
        /// <summary>
        /// Mouse button identifiers used for input queries.
        /// </summary>
        public enum Mouse
        {
            /// <summary>
            /// Left mouse button.
            /// </summary>
            kMOUSE_BUTTON_LEFT = 0,

            /// <summary>
            /// Right mouse button.
            /// </summary>
            kMOUSE_BUTTON_RIGHT = 1,

            /// <summary>
            /// Middle mouse button (usually the scroll wheel).
            /// </summary>
            kMOUSE_BUTTON_MIDDLE = 2,
        }

        private static Dictionary<Mouse, bool> mouseBtnClicked = new Dictionary<Mouse, bool>();
        private static Dictionary<KeyCode, bool> KeyClicked = new Dictionary<KeyCode, bool>();
    }
}