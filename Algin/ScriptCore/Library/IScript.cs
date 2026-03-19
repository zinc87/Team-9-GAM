using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

#pragma warning disable CS1591 // Missing XML comment for publicly visible type or member

namespace Script.Library
{
    [System.AttributeUsage(System.AttributeTargets.Field)]
  
    public class SerializeField : System.Attribute { }
    /// <summary>
    /// Base class for all user scripts attached to an engine object.
    /// </summary>
    public abstract class IScript
    {
        /// <summary>
        /// The owning engine object for this script.
        /// </summary>
        public readonly Object Obj;

        /// <summary>
        /// Called once when the script instance is created.
        /// </summary>
        public abstract void Awake();
        /// <summary>
        /// Called on the first frame the script is active.
        /// </summary>
        public abstract void Start();
        /// <summary>
        /// Called every frame with the delta time in seconds.
        /// </summary>
        public abstract void Update(double dt);
        /// <summary>
        /// Called after Update to run late-frame logic.
        /// </summary>
        public abstract void LateUpdate(double dt);
        /// <summary>
        /// Called when the script is destroyed for cleanup.
        /// </summary>
        public abstract void Free();

        public virtual void onHover()
        {

        }
        public virtual void leftOnClick()
        {

        }
        public virtual void middleOnClick()
        {

        }
        public virtual void rightOnClick()
        {

        }
        public virtual void leftOnRelease()
        {

        }
        public virtual void middleOnRelease()
        {

        }
        public virtual void rightOnRelease()
        {

        }

        public IScript()
        {   
            Obj = new Object();
        }
        /// <summary>
        /// Construct a script bound to a specific engine object ID.
        /// </summary>
        public IScript(string newID)
        {
            Obj = new Object(newID);
        }

    }
}