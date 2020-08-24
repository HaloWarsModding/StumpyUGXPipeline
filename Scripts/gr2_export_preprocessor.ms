--Script by Stumpy

fn HBM_IsBone item =
(
    return (classof item == BoneGeometry or
            classof item == Dummy or
            classof item == Biped_Object)
)

fn HideBoneMeshes =
(
	allBones =#()
    allBones = for item in objects where HBM_IsBone item == true collect item
    for bone in allBones do
    (
        GrannySetValue "Include reference in export" false mesh:bone
        GrannySetValue "Include reference in export" true bone:bone
        GrannySetValue "Include reference in export" true boneanim:bone
        GrannySetValue "Include reference in export" true modelanim:bone
    )
)

fn HBM_IsMesh item =
(
	return (classof item == Editable_mesh or
	classof item == Editable_poly or
	classof item == PolyMeshObject)
)

fn RemoveMeshesFromBones =
(
	allMeshes =#()
    allMeshes = for item in objects where HBM_IsMesh item == true collect item
	for mesh in allMeshes do
    (
        GrannySetValue "Include reference in export" false bone:mesh
        GrannySetValue "Include reference in export" false boneanim:mesh
        GrannySetValue "Include reference in export" false modelanim:mesh
    )
)

HideBoneMeshes()
RemoveMeshesFromBones() 
 
