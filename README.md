# Patch List module for IIS 7+
Generates a recursive directory listing of root suitable for patchers.

## Preview
The root URL of an application, e.g: http://localhost/x-list generates a recursive file list (in XML by default).

![XML Listing](https://imgr.es/8AGH)

The dates are in **UTC format** and the text encoding of the list is UTF-8.

The list format can also be generated as JSON with the **?json** query string as: http://localhost/x-list?json

![JSON Listing](https://imgr.es/8AGI)

## Compiling
Project was designed in Visual Studio 2017 Community and requires the C++ along with Windows SDK.

Open up the **win32\IIS_Module_PatchList.sln** file and select your target (Win32 or x64 and Debug or Release).
The output files are generated in **build\\** folder.

## Installing in IIS 7+
This will guide you through installing the module for individual applications only.
The module will still have to be "registered" globally first.

Before you begin, find a suitable location for the DLL and copy it from the build output in previous step.
Only the DLL is required, but the PDB can be copied as well for completeness in case debugging is required.

1. Open up **Internet Information Services (IIS) Manager** and click on the name of the computer or server to access global configuration for the IIS server.
2. Go to **Modules** and click on *Configure Native Modules...* in the Actions pane on the right.
3. Click on Register to add a new native or managed module. Set the name to anything you prefer, e.g. **Patch List**
4. Fill in the Path to the location of the DLL you chose earlier.
5. After clicking OK the **Patch List** module has been added **and enabled globally for every website by default** in the *Configure Native Modules* dialog.
6. Uncheck the **Patch List** in the *Configure Native Modules* dialog to disable it from being globally enabled (individual websites can then be configured to use the module).
7. Click on a website application under **Sites**, e.g: *Default Web Site* that you wish to add the module to and then click on **Modules** icon again.
8. Make sure the top URL bar says **[Server Name] > Sites > [Default Web Site]** where Default Web Site is the site you chose, so that the module is enabled for that individual website only.
9. Click on **Configure Native Modules...** again in the right pane and check the checkbox for **Patch List** to enable the module for this specific website and click OK.

The module is now enabled for that specific website, and **/x-list** path will be overriden to execute the **Patch List** module regardless of what kind of ASP.NET/Core or other framework is installed under that website "application".
