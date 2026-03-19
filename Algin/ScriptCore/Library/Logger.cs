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
    /// Simple logging facade bridging to the engine.
    /// </summary>
    public static class Logger
    {
        /// <summary>
        /// Log severity levels.
        /// </summary>
        public enum LogLevel
        {
            Trace,
            Info,
            Warning,
            Error,
            Fatal
        }
        /// <summary>
        /// Emit a log line at the given level.
        /// </summary>
        public static void log(LogLevel level, string msg)
        {
            PrivateAPI.InternalCall.log((int)level, msg);
        }
    }
}