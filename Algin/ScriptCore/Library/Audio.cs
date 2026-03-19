using PrivateAPI;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Permissions;
using System.Text;
using System.Threading.Tasks;

#pragma warning disable CS1591 // Missing XML comment for publicly visible type or member

namespace Script.Library
{
    /// <summary>
    /// Audio Instance is a wrapper that represent of each instance of the audio event of the object. Each object of each event have its own audio instance.
    /// It should only be initialized using method getAudioInstance in Audio class.
    /// </summary>
    public class AudioInstance
    {
        internal AudioInstance(string newAudioEvent, string newObjID)
        {
            audioEvent = newAudioEvent;
            obj = new Object(newObjID);
        }
        internal string audioEvent;
        internal Object obj;
    }

    /// <summary>
    /// Static Audio Library that contains functions for audio control
    /// </summary>
    public static class Audio
    {
        /// <summary>
        /// Method used for initializing AudioInstance. This should be the only way to initialize Audioinstance. Method checks for whether the audio event is
        /// added into the Audio Component in the engine. If audio event is included in the audio component, valid AudioInstance of the specific audio event is
        /// returned. Otherwise it return null
        /// </summary>
        /// <param name="eventName">
        /// Event name without any suffix/ prefix. Eg. eventName is "random" if the engine shows "event:/random" in audio component.
        /// </param>
        /// <param name="obj">
        /// Object that the audio event belongs to
        /// </param>
        /// <returns>
        /// Valid AudioInstance if exist. Null and warning message if invalid.
        /// </returns>
        /// <example>
        /// AudioInstance audioInst = Audio.getAudioInstance("random", this.Obj); //Assume it is used within INode/ IScript
        /// </example>
        static public AudioInstance getAudioInstance(string eventName, Object obj)
        {
            if (obj == null) 
            {
                Logger.log(Logger.LogLevel.Warning, $"Not valid script");
                return null;
            }

            string fullPath = InternalCall.getAudioFullPath(obj.ObjectID, eventName);
            if (fullPath != "")
            {
                return new AudioInstance(fullPath, obj.ObjectID);
            }
            Logger.log(Logger.LogLevel.Warning, $"Need to add event {eventName} into the object audio component before valid to use");
            return null;
        }
        /// <summary>
        /// Play audio event. Start audio event when not started and unpause event for paused event. Do nothing if AudioInstance is invalid/ AudioInstance
        /// is playing
        /// </summary>
        /// <param name="audioInstance">
        /// AudioInstance to be played
        /// </param>
        static public void playAudio(AudioInstance audioInstance)
        {
            if (audioInstance == null)
            {
                //Logger.log(Logger.LogLevel.Warning, $"Invalid audio instance");
                return;
            }
            InternalCall.playAudio(audioInstance.obj.ObjectID, audioInstance.audioEvent);
        }
        /// <summary>
        /// Pause audio event. Pause audio event if AudioInstancet is playing. Do nothing if AudioInstance is invalid/ AudioInstance
        /// is pausing
        /// </summary>
        /// <param name="audioInstance">
        /// AudioInstance to be paused
        /// </param>
        static public void pauseAudio(AudioInstance audioInstance)
        {
            if (audioInstance == null)
            {
                //Logger.log(Logger.LogLevel.Warning, $"Invalid audio instance");
                return;
            }
            InternalCall.pauseAudio(audioInstance.obj.ObjectID, audioInstance.audioEvent);
        }
        /// <summary>
        /// Stop audio event with FMOD default fadeout. Stop audio event if AudioInstance is playing/ pausing. Do nothing if AudioInstance is invalid/ AudioInstance
        /// is Stoping. 
        /// </summary>
        /// <param name="audioInstance">
        /// AudioInstance to be paused
        /// </param>
        static public void stopAudio(AudioInstance audioInstance)
        {
            if (audioInstance == null)
            {
                //Logger.log(Logger.LogLevel.Warning, $"Invalid audio instance");
                return;
            }
            InternalCall.stopAudio(audioInstance.obj.ObjectID, audioInstance.audioEvent);
        }
        /// <summary>
        /// Get the parameter value of the AudioInstance
        /// </summary>
        /// <param name="audioInstance">
        /// AudioInstance to be checked
        /// </param>
        /// <param name="parameterName">
        /// Name of the parameter added to the audio event in FMOD studio.
        /// </param>
        /// <returns>
        /// Value of the parameter in float.
        /// </returns>
        static public float getAudioParameter(AudioInstance audioInstance, string parameterName)
        {
            if (audioInstance == null)
            {
                //Logger.log(Logger.LogLevel.Warning, $"Invalid audio instance");
                return -1.0f;
            }
            return InternalCall.getAudioParameter(audioInstance.obj.ObjectID, audioInstance.audioEvent, parameterName);
        }

        static public void setAudioParameter(AudioInstance audioInstance, string parameterName, float value)
        {
            if (audioInstance == null)
            {
                //Logger.log(Logger.LogLevel.Warning, $"Invalid audio instance");
                return;
            }
            InternalCall.setAudioParameter(audioInstance.obj.ObjectID, audioInstance.audioEvent, parameterName, value);
        }

        static public void setVolume(string busName, float volume)
        {
            InternalCall.setVolume(volume, busName);
        }

        static public float getVolume(string busName)
        {
            return InternalCall.getVolume(busName);
        }

        static public void setMasterVolume(float volume)
        {
            InternalCall.setVolume(volume);
        } 
        static public float getMasterVolume()
        {
            return InternalCall.getVolume();
        }

        static public void setAudioInstanceVolume(AudioInstance audioInstance, float volume)
        {
            if (audioInstance == null) return;
            InternalCall.setAudioInstanceVolume(audioInstance.obj.ObjectID, audioInstance.audioEvent, volume);
        }

    }
}
