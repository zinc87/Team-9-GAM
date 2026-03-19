using Script.Library;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace PrivateAPI
{
    //3. Add C# function here
    internal static class InternalCall
    {
        #region Object
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool objectHasComponent_type(string entityID, Type componentType);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool objectHasComponent_enum(string entityID, ComponentTypes componentType);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void objectAddComponent(string entityID, ComponentTypes componentType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string createGameObject();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string[] getAllObjects();
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setGameObjectActive(string entityID, bool active);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool getGameObjectActive(string entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void linkParentChild(string parentID, string childID);
        #endregion

        #region Component
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void getTransformComponent(string objID, out Script.Library.Transformation trf);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setTransformComponent(string objID, ref Script.Library.Transformation trf);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void getCameraComponent(string objID, out Script.Library.CameraComponentData cam);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setCameraComponent(string objID, ref Script.Library.CameraComponentData cam);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void getLightComponent(string objID, out Script.Library.LightComponentData light);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setLightComponent(string objID, ref Script.Library.LightComponentData light);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void getRigidBodyComponent(string objID, out Script.Library.RigidBodyComponentData rb);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setRigidBodyComponent(string objID, ref Script.Library.RigidBodyComponentData rb);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void getBoxColliderComponent(string objID, out Script.Library.BoxColliderComponentData bc);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setBoxColliderComponent(string objID, ref Script.Library.BoxColliderComponentData bc);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void getSkyBoxComponent(string objID, out Script.Library.SkyBoxComponentData sky);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void setSkyBoxComponent(string objID, ref Script.Library.SkyBoxComponentData sky);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void getParticleComponent(string objID, out Script.Library.ParticleComponentData pc);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void setParticleComponent(string objID, ref Script.Library.ParticleComponentData pc);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void getPostProcessVolumeComponent(string objID, out Script.Library.PostProcessVolumeComponentData ppv);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void setPostProcessVolumeComponent(string objID, ref Script.Library.PostProcessVolumeComponentData ppv);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void getRectTransformComponent(string objID, out Script.Library.RectTransformComponentData rt);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void setRectTransformComponent(string objID, ref Script.Library.RectTransformComponentData rt);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void getImage2DComponent(string objID, out Image2DComponentData data);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void setImage2DComponent(string objID, ref Image2DComponentData data);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void getTextMeshUIComponent(string objID, out Script.Library.TextMeshUIComponentData tm);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void setTextMeshUIComponent(string objID, ref Script.Library.TextMeshUIComponentData tm);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void setTextMeshUIText(string objID, string text);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setFont(string objID, string font_str);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setFontSize(string objID, float size);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setLineSpacing(string objID, float spacing);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setLetterSpacing(string objID, float spacing);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setWrapLimit(string objID, float limit);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setWrap(string objID, bool wrap);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setCentered(string objID, bool centered);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setTextColor(string objID, float r, float g, float b, float a);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void getButtonComponent(string objID, out Script.Library.ButtonComponentData btn);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setButtonComponent(string objID, ref Script.Library.ButtonComponentData btn);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void getMeshRendererComponent(string objID, out Script.Library.MeshRendererComponentData meshrenderer);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void setMeshRendererComponent(string objID, ref Script.Library.MeshRendererComponentData meshrenderer);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void getSkinnedMeshRenderer(string objID, out Script.Library.SkinnedMeshRendererData skinnedmeshrenderer);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void setSkinnedMeshRenderer(string objID, ref Script.Library.SkinnedMeshRendererData skinnedmeshrenderer);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void getVideoPlayerComponent(string objID, out Script.Library.VideoPlayerComponentData vp);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void setVideoPlayerComponent(string objID, ref Script.Library.VideoPlayerComponentData vp);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void playVideo(string objID, string path);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void stopVideo(string objID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool isVideoFinished(string objID);

        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setMeshFromString(string objID, string meshName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setMeshFromHash(string objID, ulong hash);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void changeModel(string objID, string modelName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void changeShaderExcept(string objID, string shaderName, string submeshName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void playAnimation(string objID, string animationName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void setLoopAnimation(string objID, bool shouldLoop);

        /*[MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void stopAnimation(string objID);*/

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void playCurrentAnimation(string objID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void stopAnimation(string objID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void resetAnimationTimer(string objID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setAnimationTimer(string objID, float time);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setSubmeshMaterial(string objID, string submesh_name, string material_name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setMeshRendererSubmeshMaterial(string objID, string submeshName, string materialName);

        #endregion

        #region Camera

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GetCameraRay(string objID, out CameraRay ray);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setOverlayCamera(string objID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setOverlayActive(bool active);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setOverlayViewport(float x, float y, float w, float h);

        #endregion

        #region Logger
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void log(int level, string logmsg);
        #endregion

        #region Debug Drawing
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void drawLine(Vector3D a, Vector3D b);

        #endregion

        #region Input
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void getMousePostion(out Script.Library.Vector2D mousePos);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void getMouseDelta(out Script.Library.Vector2D mouseDelta);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void mouseScroll(out Script.Library.Vector2D offset);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool isMousePressed(int btn);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool isMouseReleased(int btn);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool isKeyPressed(int key);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool isKeyReleased(int key);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool getMouseState();
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setMouseState(bool state);
        #endregion

        #region BHT
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setNodeStatus(string objID, string nodeID, NodeStatus status);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void getNodeStatus(string objID, string nodeID, out NodeStatus status);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setNodeResult(string objID, string nodeID, NodeResult result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void getNodeResult(string objID, string nodeID, out NodeResult result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void nodeTick(string objID, string nodeID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string[] getNodeChildrens(string objID, string nodeID);

        #endregion

        #region Audio
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string getAudioFullPath(string objID, string eventName);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void playAudio(string objID, string eventName);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void pauseAudio(string objID, string eventName);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void stopAudio(string objID, string eventName);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float getAudioParameter(string objID, string eventName, string paramName);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setAudioParameter(string objID, string eventName, string paramName, float value);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setVolume(float volume, string busName = "");
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float getVolume(string busName = "");

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setAudioInstanceVolume(string objID, string eventName, float volume);

        #endregion

        #region Physics
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool isRayCastHit(Vector3D origin, Vector3D direction, out RaycastHitDataInternal hitInfo, float maxDistance);
        #endregion

        #region Scene
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void loadScene(string fileName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string getSceneName();
        #endregion

        #region temp
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void setScanState(bool state);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool getScanState();
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string createPatient();
        #endregion

        // for gamma correction
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float getGamma();

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void setGamma(float gamma);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool getTonemapEnabled();

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void setTonemapEnabled(bool enabled);
    }
}
