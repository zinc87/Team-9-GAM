using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Script.Library
{
    public class Image2D
    {
        public Image2D()
        {
            hashedID = 0;
        }
        public Image2D(ulong newID)
        {
            hashedID = newID;
        }
        public ulong hashedID { get; private set; }
    }
}
