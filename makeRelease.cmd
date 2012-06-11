REM .\bin\Release\pbrt.exe --ncores 1 render_physics.pbrt && .\bin\Release\exrtotiff.exe project.exr project.tiff && explorer project.tiff
.\bin\Release\pbrt.exe --ncores 1 %1.pbrt && .\bin\Release\exrtotiff.exe %1.exr %1.tiff && explorer %1.tiff

