using PrivateAPI;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Script.Library
{
    public static class Scan
    {
        static public void setScan(bool state)
        {
            InternalCall.setScanState(state);
        }

        static public bool getScan()
        {
            return InternalCall.getScanState();
        }
    }
}
