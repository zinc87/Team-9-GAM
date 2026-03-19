using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static Script.Library.Input;

#pragma warning disable CS1591 // Missing XML comment for publicly visible type or member

namespace Script.Library
{
    /// <summary>
    /// Object is the reference back to the object in game engine. It is using ObjectID as identifier to link both C# and C++ object
    /// </summary>
    public class Object
    {
        /// <summary>
        /// Unique identifier assigned by the engine.
        /// </summary>
        public readonly string ObjectID;

        /// <summary>
        /// Default constructor; creates a placeholder object.
        /// </summary>
        public Object()
        {
            ObjectID = "0000";
        }
        /// <summary>
        /// Construct with a specific engine object ID.
        /// </summary>
        public Object(string newID)
        {
            ObjectID = newID;
        }
        /// <summary>
        /// Checks whether this object has a component of type T.
        /// </summary>
        public bool hasComponent<T>() where T : IComponent, new()
        {
            Type componentType = typeof(T);
            return PrivateAPI.InternalCall.objectHasComponent_type(ObjectID, componentType);
        }
        /// <summary>
        /// Checks whether this object has a component of type T but using the enum compoent type as parameter.
        /// </summary>
        public bool hasComponent(ComponentTypes type)
        {
            return PrivateAPI.InternalCall.objectHasComponent_enum(ObjectID, type);
        }

        /// <summary>
        /// Add component if the object dont have the component.
        /// <parameter> types
        /// enum component type that should match the enum component type in the engine
        /// </parameter>
        /// </summary>
        public void addComponent(ComponentTypes types)
        {
            if (!hasComponent(types))
            {
                PrivateAPI.InternalCall.objectAddComponent(ObjectID, types);
            }
        }

        public bool isActive()
        {
            return PrivateAPI.InternalCall.getGameObjectActive(ObjectID);
        }

        public void setActive(bool active)
        {
            PrivateAPI.InternalCall.setGameObjectActive(ObjectID, active);
        }

        //public TransformComponent getComponent()
        //{
        //    new TransformComponent().getter();
        //    return 
        //}

        /// <summary>
        /// Gets a component of type T if present; otherwise returns null.
        /// </summary>
        public T getComponent<T>() where T : IComponent, new()
        {
            if (!hasComponent<T>())
                return null;
            else
            {
                return new T() { Obj = this };
            }
        }
        public static string createGameObject()
        {
            return PrivateAPI.InternalCall.createGameObject();
        }

        public static string createPatient()
        {
            return PrivateAPI.InternalCall.createPatient();
        }

        public static string[] GetAllObjects()
        {
            return PrivateAPI.InternalCall.getAllObjects();
        }

        /// <summary>
        /// Link a child object to this parent object.
        /// </summary>
        public void AddChild(Object child)
        {
            if (child != null)
            {
                PrivateAPI.InternalCall.linkParentChild(this.ObjectID, child.ObjectID);
            }
        }

        /// <summary>
        /// Create a new empty child object and link it to this parent.
        /// </summary>
        public Object CreateChild()
        {
            string childID = PrivateAPI.InternalCall.createGameObject();
            PrivateAPI.InternalCall.linkParentChild(this.ObjectID, childID);
            return new Object(childID);
        }
    }
}