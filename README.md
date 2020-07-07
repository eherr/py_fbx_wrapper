# py_fbx_wrapper
Simple Cython wrapper for the FBX SDK to load characters with meshes into a dictionary structure. It was tested using [vis_utils](https://github.com/eherr/vis_utils) with files from [MakeHuman](http://www.makehumancommunity.org/) but might also work with files from other sources.

You need to change path to the python environment in the files FBXWrapper.vcxproj and FBXImporterWrapper.vcxproj 

Additionally the following dependencies are required:
- [GLM](https://glm.g-truc.net/0.9.9/index.html)
- [FBX SDK](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-0) (It was tested with version 2017)

To generate the C++ wrapper from fbx_importer.pyx, [Cython](https://cython.org/) needs to be installed 
```bat
pip cython
```

Cython can then convert fbx_importer.pyx into fbx_importer.cpp which is then compiled into fbx_importer.pyd and linked with FBXImporter.lib.
```bat
<Path to Python Environment>\Scripts\cython.exe fbx_importer.pyx --cplus
```

To automate this step, a custom pre-build event is defined in the project file FBXImporterWrapper.vcxproj for which only the path to the environment needs to be changed.

Note the module fbx_importer.pyd can only be imported by a python script if libfbxsdk.dll is in the same directory. The projects are only configured for Release|x64 because Python does not come with debug files for Windows.

```bat
import fbx_importer

data = fbx_importer.load_fbx_file(filename)

```
Data contains a "skeleton", "animations" and a "mesh_list". Each entry of the mesh list contains with vertices, normals, uvs, bone ids and weights. Each animation contains a "curves" dict that stores the joint names as keys and a list of frames with "local_translation" and "local_rotation" as keys.
 
## License
Copyright (c) 2019 DFKI GmbH.  
MIT License, see the LICENSE file.  