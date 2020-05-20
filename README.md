# py_fbx_wrapper
Simple Cython wrapper for the FBX SDK for characters with meshes. It was mainly tested with files from  [MakeHuman](http://www.makehumancommunity.org/) but might also work with files from other sources.

You need to change path to the python environment in the files FBXWrapper.csproj and FBXImporterWrapper.csproj 

Additionally the following dependencies are required:
- [GLM]https://glm.g-truc.net/0.9.9/index.html)
- [FBX SDK] https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-0 (It was tested with version 2017)

To generate the C++ wrapper from fbx_importer.pyx, [Cython](https://cython.org/) needs to be installed 
```bat
pip cython
```

Cython can then convert fbx_importer.pyx into fbx_importer.cpp which is then compiled into fbx_importer.pyd that is linked with FBXImporter.lib can be imported by a python script.
```bat
<Path to Python Environment>\Scripts\cython.exe fbx_importer.pyx --cplus
```

To automate this step, a custom pre-build event is defined in the project file FBXImporterWrapper.csproj for which only the path to the environment needs to be changed.

 
## License
Copyright (c) 2019 DFKI GmbH.  
MIT License, see the LICENSE file.  