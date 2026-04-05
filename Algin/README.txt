Build Instructions: Algin / Silent Haze

=== Prerequisites ===
* Visual Studio (with MSBuild)
* .NET Framework 4.7.2 Developer Pack (Strict requirement for the ScriptCore project)

---

=== Option 1: Command Line Build (Batch Script) *RECOMMENDED* ===
Run the provided Jenkins batch script with your desired configuration to automatically build the solution using MSBuild.

Available Configurations:
* DEBUG
* RELEASE
* EXPORT
* ALL

Usage:
Run JENKINS_<Configuration>.bat (e.g., JENKINS_RELEASE.bat). 

Note on MSBuild Location: 
The script searches for vswhere.exe at the default Visual Studio installer location:
C:\Program Files (x86)\Microsoft Visual Studio\Installer
If your installation is on a different drive or path, either edit lines 4 & 5 of the .bat script to match your local system, or proceed to Option 2.



=== Option 2: Visual Studio Build (IDE) === 
1. Open Algin.sln in Visual Studio.
2. Right-click the Silent Haze project and select "Set as Startup Project".
3. Verify your Project Dependencies/Build Order to ensure that Silent Haze is the last project to build.
4. Go to the top menu and select Build > Rebuild All.


=== Troubleshooting & Known Issues ===
ScriptCore .NET Framework Mismatch
The ScriptCore project will not compile without .NET Framework 4.7.2. If you encounter load errors, or if you accidentally selected 4.8 when loading the project, follow these steps to fix it:

1. Right-click the ScriptCore project in the Solution Explorer and select "Edit Project File".
2. Locate the <TargetFrameworkVersion> line and change its value to v4.7.2.
3. Save the file, reload the ScriptCore project, and choose the 4.7.2 option.
4. If you do not have it installed: Download the .NET 4.7.2 Developer Pack from your browser, install it, relaunch Visual Studio, and build the solution again.

Build Failures by Configuration
* Release Mode: If there is a build error on the first try, just build it again. It might be a post-build command or parallel PDB error.
* Export Mode: Ensure your platform is set strictly to "x64", not "Any CPU".

=== Build Output Location ===
* All builds will be located in: bin/x64/<Configuration>