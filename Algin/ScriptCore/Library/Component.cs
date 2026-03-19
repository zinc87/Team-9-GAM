using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

#pragma warning disable CS1591 // Missing XML comment for publicly visible type or member

namespace Script.Library
{
    public enum ComponentTypes
    {
        None = 0,
		Transform,
		RectTransform,
		Camera,
		Texture,
		Light,
		Audio,
		MeshRenderer,
		SkinnedMeshRenderer,
		SkyBox,
		BoxCollider,
		PostProcessVolume,
		RigidBody,
		Image2DComponent,
		Script,
		Prefab,
		TextMeshUI,
		TextMesh,
		Particles,
		Behavior,
		Cube,
		Button,
		CapsuleCollider,
        VideoPlayer,
	};

    /// <summary>
    /// Base interface for all script components.
    /// </summary>
    public abstract class IComponent
    {
        /// <summary>
        /// The owning engine object for this component.
        /// </summary>
        public Object Obj { get; set; }
    }

    [StructLayout(LayoutKind.Sequential)]

    public struct Transformation
    {
        /// <summary>
        /// World-space position.
        /// </summary>
        public Vector3D position;
        /// <summary>
        /// Euler rotation (degrees).
        /// </summary>
        public Vector3D Scale;
        /// <summary>
        /// Local scale.
        /// </summary>
        public Vector3D Rotation;

        public Vector3D getForward()
        {
            Vector3D rad = Rotation * (Math.PI / 180.0);

            float cx = (float)Math.Cos(rad.x);
            float sx = (float)Math.Sin(rad.x);
            float cy = (float)Math.Cos(rad.y);
            float sy = (float)Math.Sin(rad.y);

            // Unity-style forward (Z+)
            return new Vector3D(
                sy,
                -sx * cy,
                cx * cy
            ).normalize();
        }
        public Vector3D getFlatForward()
        {
            float yawRad = Rotation.y * (float)Math.PI / 180f;
            return new Vector3D(
                (float)Math.Sin(yawRad),  //X
                0f,                       // Y
                -(float)Math.Cos(yawRad)   // Z
            ).normalize();
        }

        public Vector3D getBackward()
        {
            return -getForward();
        }

        public Vector3D getFlatBackward()
        {
            return -getFlatForward();
        }

        public Vector3D getRight()
        {
            Vector3D rad = Rotation * (Math.PI / 180.0);

            float cx = (float)Math.Cos(rad.x);
            float sx = (float)Math.Sin(rad.x);
            float cy = (float)Math.Cos(rad.y);
            float sy = (float)Math.Sin(rad.y);

            return new Vector3D(
                cy,
                sx * sy,
                -cx * sy
            ).normalize();
        }
        public Vector3D getFlatRight()
        {
            float yawRad = Rotation.y * (float)Math.PI / 180f;
            return new Vector3D(
                (float)Math.Cos(yawRad),  // X
                0f,                       // Y
                (float)Math.Sin(yawRad)  // Z
            ).normalize();
        }

        public Vector3D getLeft()
        {
            return -getRight();
        }

        public Vector3D getFlatLeft()
        {
            return -getFlatRight();
        }

        public Vector3D getUp()
        {
            Vector3D rad = Rotation * (Math.PI / 180.0);

            float cx = (float)Math.Cos(rad.x);
            float sx = (float)Math.Sin(rad.x);
            float cy = (float)Math.Cos(rad.y);
            float sy = (float)Math.Sin(rad.y);

            return new Vector3D(
                0,
                cx,
                sx
            ).normalize();
        }

        public Vector3D getDown()
        {
            return -getUp();
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct CameraComponentData
    {
        public float near;
        public float far;
        public float fov;
        public float vertical;
        public int proj;
        [MarshalAs(UnmanagedType.I1)]
        public bool isGameCam;
    }
    

    [StructLayout(LayoutKind.Sequential)]
    public struct LightComponentData
    {
        public Vector3D position;
        public Vector3D direction;
        public float intensity;
        public float range;
        public int type_index;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct RigidBodyComponentData
    {
        public Vector3D drag;
        public Vector3D velocity;
        public Vector3D acceleration;
        public Vector3D force;
        public float mass;
        public Vector3D gravity;
        [MarshalAs(UnmanagedType.I1)]
        public bool useGravity;
        public Vector3D finalAcceleration;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct BoxColliderComponentData
    {
        public Vector3D center_offset;
        public Vector3D halfExtent;
        [MarshalAs(UnmanagedType.I1)]
        public bool isTrigger;
        [MarshalAs(UnmanagedType.I1)]
        public bool drawCollider;
        public uint layerMask;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SkyBoxComponentData
    {
        public int cm_index;
        public float exposure;
        public Vector3D tint;
        public Vector3D rotation;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ParticleComponentData
    {
        public Vector3D position;
        public float speed;
        public float upVel;
        public float grav;
        public bool play;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct PostProcessVolumeComponentData
    {
        public bool isGlobal;
        public Vector3D VolCentre;
        public Vector3D HalfExtend;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct RectTransformComponentData
    {
        public Vector2D position;
        public Vector2D scale;
        public float rotation;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Color
    {
        public float r;
        public float g;
        public float b;
        public float a;

        public Color(float r, float g, float b, float a = 1.0f)
        {
            this.r = r;
            this.g = g;
            this.b = b;
            this.a = a;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct TextureCoordinate
    {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
        public float[] textureCoord; // 4x2 = 8 floats

        public TextureCoordinate(bool initialize = true)
        {
            textureCoord = new float[8];
            if (initialize)
            {
                textureCoord[0] = 0f; textureCoord[1] = 1f;
                textureCoord[2] = 1f; textureCoord[3] = 1f;
                textureCoord[4] = 1f; textureCoord[5] = 0f;
                textureCoord[6] = 0f; textureCoord[7] = 0f;
            }
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Image2DComponentData
    {
        public ulong hashed;
        public Vector2D topLeft;
        public Vector2D topRight;
        public Vector2D bottomLeft;
        public Vector2D bottomRight;
        public Color color;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ButtonComponentData
    {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public int[] L_buttonStates; // 0 or 1
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public int[] R_buttonStates;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public int[] M_buttonStates;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct TextMeshUIComponentData
    {
        public Color color;
        public ulong hashed;

        public string text;

        public float size;
        public float spacing;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct MeshRendererComponentData
    {
        public ulong model_hash;
        public string mesh_name;
        //public ulong submeshNameHashes;
        //public ulong materialHashList;
        //public Vector3D materialValueList;
        public float shininess;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SkinnedMeshRendererData
    {
        public ulong skinned_mesh_hash;
        public string animation_clip_name;
        public bool loop;
        public bool play;
    }

    public class TransformComponent : IComponent
    {
        /// <summary>
        /// Gets/sets the object's transformation.
        /// </summary>
        public Transformation Transformation
        {
            get
            {
                PrivateAPI.InternalCall.getTransformComponent(Obj.ObjectID, out Transformation trf);
                return trf;
            }
            set
            {
                PrivateAPI.InternalCall.setTransformComponent(Obj.ObjectID, ref value);
            }
        }
    }

    public class CameraComponent : IComponent
    {
        /// <summary>
        /// Gets/sets the object's camera component.
        /// </summary>
        public CameraComponentData Data
        {
            get
            {
                PrivateAPI.InternalCall.getCameraComponent(Obj.ObjectID, out CameraComponentData cam);
                return cam;
            }
            set
            {
                PrivateAPI.InternalCall.setCameraComponent(Obj.ObjectID, ref value);
            }
        }
    }

    public class LightComponent : IComponent
    {
        /// <summary>
        /// Gets/sets the object's light component.
        /// </summary>
        public LightComponentData Data
        {
            get
            {
                PrivateAPI.InternalCall.getLightComponent(Obj.ObjectID, out LightComponentData data);
                return data;
            }
            set
            {
                PrivateAPI.InternalCall.setLightComponent(Obj.ObjectID, ref value);
            }
        }
    }

    public class BoxColliderComponent : IComponent
    {
        /// <summary>
        /// Gets/sets the object's collision.
        /// </summary>
        public BoxColliderComponentData Data
        {
            get
            {
                PrivateAPI.InternalCall.getBoxColliderComponent(Obj.ObjectID, out BoxColliderComponentData data);
                return data;
            }
            set
            {
                PrivateAPI.InternalCall.setBoxColliderComponent(Obj.ObjectID, ref value);
            }
        }
    }

    public class RigidBodyComponent : IComponent
    {
        /// <summary>
        /// Gets/sets the object's physics.
        /// </summary>
        public RigidBodyComponentData Data
        {
            get
            {
                PrivateAPI.InternalCall.getRigidBodyComponent(Obj.ObjectID,out RigidBodyComponentData rb);
                return rb;
            }
            set
            {
                PrivateAPI.InternalCall.setRigidBodyComponent(Obj.ObjectID,ref value);
            }
        }
    }


    public class SkyBoxComponent : IComponent
    {
        /// <summary>
        /// Gets/sets the skybox component.
        /// </summary>
        public SkyBoxComponentData Data
        {
            get
            {
                PrivateAPI.InternalCall.getSkyBoxComponent(Obj.ObjectID, out SkyBoxComponentData sky);
                return sky;
            }
            set
            {
                PrivateAPI.InternalCall.setSkyBoxComponent(Obj.ObjectID, ref value);
            }
        }

    }

    public class ParticleComponent : IComponent
    {
        /// <summary>
        /// Gets/sets the particle component.
        /// </summary>
        public ParticleComponentData Data
        {
            get
            {
                PrivateAPI.InternalCall.getParticleComponent(Obj.ObjectID, out ParticleComponentData pc);
                return pc;
            }
            set
            {
                PrivateAPI.InternalCall.setParticleComponent(Obj.ObjectID, ref value);
            }
        }

    }

    public class PostProcessVolumeComponent : IComponent
    {
        /// <summary>
        /// Gets/sets the post process volume component.
        /// </summary>
        public PostProcessVolumeComponentData Data
        {
            get
            {
                PrivateAPI.InternalCall.getPostProcessVolumeComponent(Obj.ObjectID, out PostProcessVolumeComponentData ppv);
                return ppv;
            }
            set
            {
                PrivateAPI.InternalCall.setPostProcessVolumeComponent(Obj.ObjectID, ref value);
            }
        }

    }

    public class RectTransformComponent : IComponent
    {
        /// <summary>
        /// Gets/sets the rect transform component.
        /// </summary>
        public RectTransformComponentData Data
        {
            get
            {
                PrivateAPI.InternalCall.getRectTransformComponent(Obj.ObjectID, out RectTransformComponentData rt);
                return rt;
            }
            set
            {
                PrivateAPI.InternalCall.setRectTransformComponent(Obj.ObjectID, ref value);
            }
        }
    }

    public class ButtonComponent : IComponent
    {
        public ButtonComponentData Data
        {
            get
            {
                PrivateAPI.InternalCall.getButtonComponent(Obj.ObjectID, out ButtonComponentData data);
                return data;
            }
            set
            {
                PrivateAPI.InternalCall.setButtonComponent(Obj.ObjectID, ref value);
            }
        }
    }

    public class AudioComponent : IComponent
    {

    }

    public class MeshRendererComponent : IComponent
    {
        /// <summary>
        /// Gets/sets the mesh renderer component.
        /// </summary>
        public MeshRendererComponentData Data
        {
            get
            {
                PrivateAPI.InternalCall.getMeshRendererComponent(Obj.ObjectID, out MeshRendererComponentData meshrenderer);
                return meshrenderer;
            }
            set
            {
                PrivateAPI.InternalCall.setMeshRendererComponent(Obj.ObjectID, ref value);
            }

        }

        public void SetMeshRendererSubmeshMaterial(string submeshName, string materialName)
        {
            PrivateAPI.InternalCall.setMeshRendererSubmeshMaterial(Obj.ObjectID, submeshName, materialName);
        }

        public void SetMesh(string meshName)
        {
            PrivateAPI.InternalCall.setMeshFromString(Obj.ObjectID, meshName);
        }

        public void SetMesh(ulong hash)
        {
            PrivateAPI.InternalCall.setMeshFromHash(Obj.ObjectID, hash);
        }
    }

    public class TextMeshUIComponent : IComponent
    {
        /// <summary>
        /// Gets/sets the text mesh UI component.
        /// </summary>
        public TextMeshUIComponentData Data
        {
            get
            {
                PrivateAPI.InternalCall.getTextMeshUIComponent(Obj.ObjectID, out TextMeshUIComponentData tm);
                return tm;
            }
            set
            {
                PrivateAPI.InternalCall.setTextMeshUIComponent(Obj.ObjectID, ref value);
            }
        }

        public void SetFont(string font) => PrivateAPI.InternalCall.setFont(Obj.ObjectID, font);
        public void SetFontSize(float size) => PrivateAPI.InternalCall.setFontSize(Obj.ObjectID, size);
        public void SetLineSpacing(float spacing) => PrivateAPI.InternalCall.setLineSpacing(Obj.ObjectID, spacing);
        public void SetLetterSpacing(float spacing) => PrivateAPI.InternalCall.setLetterSpacing(Obj.ObjectID, spacing);
        public void SetWrapLimit(float limit) => PrivateAPI.InternalCall.setWrapLimit(Obj.ObjectID, limit);
        public void SetWrap(bool wrap) => PrivateAPI.InternalCall.setWrap(Obj.ObjectID, wrap);
        public void SetCentered(bool centered) => PrivateAPI.InternalCall.setCentered(Obj.ObjectID, centered);
        public void SetColor(float r, float g, float b, float a)
        {
            PrivateAPI.InternalCall.setTextColor(Obj.ObjectID, r, g, b, a);
        }
    }

    public class SkinnedMeshRenderer : IComponent
    {
        /// <summary>
        /// Gets/sets the skinned mesh renderer.
        /// </summary>
        public SkinnedMeshRendererData Data
        {
            get
            {
                PrivateAPI.InternalCall.getSkinnedMeshRenderer(Obj.ObjectID, out SkinnedMeshRendererData skinnedmeshrenderer);
                return skinnedmeshrenderer;
            }
            set
            {
                PrivateAPI.InternalCall.setSkinnedMeshRenderer(Obj.ObjectID, ref value);
            }
        }

        public void ChangeModel(string modelName) =>
        PrivateAPI.InternalCall.changeModel(Obj.ObjectID, modelName);

        public void ChangeShaderExcept(string shaderName, string submeshName) =>
            PrivateAPI.InternalCall.changeShaderExcept(Obj.ObjectID, shaderName, submeshName);

        public void PlayAnimation(string animationName) =>
            PrivateAPI.InternalCall.playAnimation(Obj.ObjectID, animationName);

        public void SetLoopAnimation(bool shouldLoop) =>
            PrivateAPI.InternalCall.setLoopAnimation(Obj.ObjectID, shouldLoop);

        /*public void StopAnimation() =>
            PrivateAPI.InternalCall.stopAnimation(Obj.ObjectID);*/

        public void Play()
        {
            PrivateAPI.InternalCall.playCurrentAnimation(Obj.ObjectID);
        }

        public void Stop()
        {
            PrivateAPI.InternalCall.stopAnimation(Obj.ObjectID);
        }

        public void ResetTimer()
        {
            PrivateAPI.InternalCall.resetAnimationTimer(Obj.ObjectID);
        }

        public void SetTimer(float time)
        {
            PrivateAPI.InternalCall.setAnimationTimer(Obj.ObjectID, time);
        }

        public void SetSubmeshMaterial(string submesh_name, string material_name)
        {
            PrivateAPI.InternalCall.setSubmeshMaterial(Obj.ObjectID, submesh_name, material_name);

        }

    }

    public class Image2DComponent : IComponent
    {
        /// <summary>
        /// Gets/sets the Image2D component data.
        /// </summary>
        public Image2DComponentData Data
        {
            get
            {
                PrivateAPI.InternalCall.getImage2DComponent(Obj.ObjectID, out Image2DComponentData data);
                return data;
            }
            set
            {
                PrivateAPI.InternalCall.setImage2DComponent(Obj.ObjectID, ref value);
            }
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VideoPlayerComponentData
    {
        [MarshalAs(UnmanagedType.LPStr)]
        public string videoPath;
        [MarshalAs(UnmanagedType.I1)]
        public bool playOnStart;
        [MarshalAs(UnmanagedType.I1)]
        public bool looping;
    }

    public class VideoPlayerComponent : IComponent
    {
        public VideoPlayerComponentData Data
        {
            get
            {
                PrivateAPI.InternalCall.getVideoPlayerComponent(Obj.ObjectID, out VideoPlayerComponentData data);
                return data;
            }
            set
            {
                PrivateAPI.InternalCall.setVideoPlayerComponent(Obj.ObjectID, ref value);
            }
        }

        public void Play(string path)
        {
            PrivateAPI.InternalCall.playVideo(Obj.ObjectID, path);
        }

        public void Stop()
        {
            PrivateAPI.InternalCall.stopVideo(Obj.ObjectID);
        }

        public bool IsFinished()
        {
            return PrivateAPI.InternalCall.isVideoFinished(Obj.ObjectID);
        }
    }
}
