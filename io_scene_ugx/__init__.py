import os
import bpy
import bpy_extras
import tempfile
import sys
import subprocess
import shutil
from bpy.props import FloatVectorProperty
from shutil import copyfile
from bpy.props import IntProperty, CollectionProperty, StringProperty
import xml.etree.ElementTree as ET
import xml.dom.minidom
from bpy.types import Panel, UIList
from bpy.props import (
        BoolProperty,
        FloatProperty,
        StringProperty,
        EnumProperty,
        )
from bpy_extras.io_utils import (
        ImportHelper,
        ExportHelper,
        orientation_helper,
        path_reference_mode,
        axis_conversion,
      )

bl_info = {
    "name": "HWDE Universal Geometry",
    "author": "StumpyHWDE",
    "version": (1, 0, 0),
    "blender": (2, 91, 0),
    "location": "File > Import-Export",
    "description": "Export UGX/UAX",
    "warning": "",
    "category": "Import-Export",
}

#
#
#

# UGX Export Section

def ExportUGX(self, context, ugxPath, option_exportMeshes, option_selectionOnly, option_anim_exportAnims, option_anim_allAnims, front, right, up):
    scnOld = context.scene
    scnNow = context.scene.copy()
    context.window.scene = scnNow
    
    tmpDir = tempfile.mkdtemp()
    
    
    tp1 = os.path.realpath(__file__)
    toolPath = os.path.dirname(tp1) + "\\tool"
    if option_exportMeshes == True:
        saveMatsToXML(self, context, tmpDir)
        bpy.ops.wm.collada_export(filepath=tmpDir + "\\tmp.dae", 
            use_object_instantiation=False, 
            use_blender_profile=False, 
            selected=option_selectionOnly, 
            apply_modifiers=True, 
            include_animations=False, 
            include_all_actions=False)
        tools = (
            r'"' + toolPath + r'\DAEtoGR2.exe" -debug "' + tmpDir + r'\tmp.dae"',
            "\"" + toolPath + "\\StumpyUGXSDK.exe\" -auto -mesh \"" + tmpDir + "\\tmp.gr2\" \"" + ugxPath + "\"", )
        args = (
            "-nodebug ",
            r'"' +  tmpDir + r'\tmp.dae" ',
            r'"' +  tmpDir + r'\tmp.gr2" ', )
        subprocess.call([toolPath + "\\DAEtoGR2.exe", args], shell=True)
        args = (
            "-auto ",
            "-mesh ",
            r'"' + tmpDir + r'\tmp.gr2" ',
            r'"' + ugxPath + r'" ',
            r'"' + front + r'" ',
            r'"' + right + r'" ',
            r'"' + up + r'" ',)
        subprocess.call([toolPath + "\\gr2ugx.exe", args], shell=True)
    
    
    if option_anim_exportAnims == True:
        
        cur = self.selectedArmature
        actionList = []
        
        if self.animsExportAll == True:
            for a in bpy.data.actions:
                actionList.append(a)
                
        if self.animsExportAll == False:
            if scnNow.objects[cur].animation_data is not None:
                if scnNow.objects[cur].animation_data.action is not None:
                    actionList.append(scnNow.objects[cur].animation_data.action)
                
                
        for a in actionList:
            
            activeObjects = context.selected_objects
            for obj in activeObjects:
                obj.select_set(False)
            scnNow.objects[cur].select_set(True)
            
            if scnNow.objects[cur].animation_data is None:
                scnNow.objects[cur].animation_data_create()
            
            
            scnNow.objects[cur].animation_data.action = a
            frame0 = int(a.frame_range[0])
            frameL = int(a.frame_range[1])
            
            scnNow.frame_start = frame0
            scnNow.frame_end = frameL
            print(frame0)
            print(frameL)
            print(scnNow.frame_end)
            
            
#            bpy.ops.nla.bake(
 #           frame_start=frame0,
  #          frame_end=frameL, 
   #         step=1, 
    #        only_selected=False, 
     #       visual_keying=True, 
      #      clear_constraints=False, 
       #     clear_parents=False, 
        #    use_current_action=True,
         #   bake_types={'POSE'})
            
            
            
            bpy.ops.wm.collada_export(filepath=tmpDir + "\\" + a.name + "_tmp.dae",
            use_object_instantiation=False,
            use_blender_profile=False,
            selected=True,
            apply_modifiers=True, 
            include_animations=True,
            include_all_actions=False,
            deform_bones_only=True,
            export_animation_type_selection='sample',
            sampling_rate=1,
            #keep_flat_curves=True
            )
            
            args = (
                "-debug ",
                r'"' +  tmpDir + "\\" + a.name + "_tmp.dae\" ",
                r'"' +  tmpDir + "\\" + a.name + "_tmp.gr2\" ", )
            subprocess.call([toolPath + "\\DAEtoGR2.exe", args], shell=True)
            
            
            args = (
                "-auto ",
                "-anim ",
                r'"' +  tmpDir + "\\" + a.name + "_tmp.gr2\" ",
                r'"' + os.path.dirname(ugxPath) + "\\" + a.name + ".uax\" " , 
                r'"' + front + r'" ',
                r'"' + right + r'" ',
                r'"' + up + r'" ',)
        
            subprocess.call([toolPath + "\\gr2ugx.exe", args], shell=True)
            
            
            for obj in activeObjects:
                obj.select_set(True)
    
    #print(tmpDir)
    shutil.rmtree(tmpDir)
    
    bpy.ops.scene.delete()
    context.window.scene = scnOld
    
    return;
    
    
class ExportUGXClass(bpy.types.Operator, ExportHelper):
    """Export Halo Wars: DE mesh and animation files"""
    bl_idname = "export_scene.ugx"
    bl_label = "Export UGX"
    bl_options = {'REGISTER', 'UNDO'}
    filename_ext = ".ugx"
    filter_glob: StringProperty( default='*.ugx', options={'HIDDEN'} )

    #Option Functions#
    def getAllArmaturesFromAll(self, context):
        arms = []
        
        if self.selectedOnly == False:
            for obj in context.scene.objects:
                if obj.type == 'ARMATURE':
                    arms.append((obj.name, obj.name, ""))
            pass
                    
        if self.selectedOnly == True:
            for obj in context.selected_objects:
                if obj.type == 'ARMATURE':
                    arms.append((obj.name, obj.name, ""))
            pass
                    
        if len(arms) == 0:
            arms.append(('NONE', 'NONE', ""))
        return arms

    #Options#
    exportMeshes: bpy.props.BoolProperty(
            name="Export Meshes",
            description="Export meshes into a .ugx file",
            default=True,
            )
    
    selectedArmature: EnumProperty(
            name="",
            description="Select the armature that will be included in the .ugx file. This armature will also be the source for all .uax animations",
            items=getAllArmaturesFromAll,
            )
    
    selectedOnly: bpy.props.BoolProperty(
            name="Selected Only",
            description="Only export objects that are selected",
            default=True,
            )
            
    animsEnable: bpy.props.BoolProperty(
            name="Export Animations",
            description="Export animations to separate .uax files",
            default=False,
            )
            
    animsExportAll: bpy.props.BoolProperty(
            name="All Actions",
            description="Export each action on the selected armature to a separate .uax file",
            default=True,
            )

    orientationBack: bpy.props.EnumProperty(
            name="Back", 
            items={('X+', 'X+', ''),('Y+', 'Y+', ''),('Z+', 'Z+', ''),('X-', 'X-', ''),('Y-', 'Y-', ''),('Z-', 'Z-', '')}, 
            default='Z-'
    )
    orientationRight: bpy.props.EnumProperty(
            name="Right", 
            items={('X+', 'X+', ''),('Y+', 'Y+', ''),('Z+', 'Z+', ''),('X-', 'X-', ''),('Y-', 'Y-', ''),('Z-', 'Z-', '')}, 
            default='X-'
    )
    orientationUp: bpy.props.EnumProperty(
            name="Up", 
            items={('X+', 'X+', ''),('Y+', 'Y+', ''),('Z+', 'Z+', ''),('X-', 'X-', ''),('Y-', 'Y-', ''),('Z-', 'Z-', '')}, 
            default='Y+'
    )

    #GUI#
    def draw(self, context):
        layout = self.layout

        boxO = layout.box()
        boxO.label(text="Export Settings", icon='SETTINGS')
        
        boxGen = boxO.box()
        boxGen.label(text="General", icon='OPTIONS')
        
        
        colL = boxGen.split(factor=0.5)
        colL.prop(self, "orientationBack")
        colL.prop(self, "orientationRight")
        boxGen.prop(self, "orientationUp")
        
        
        boxGen.prop(self, "exportMeshes")
        boxGen.prop(self, "selectedOnly")
        
        box = boxGen.box()
        box.label(text="Selected Armature")
        box.prop(self, "selectedArmature")
        
        
        boxAnim = boxO.box()
        boxAnim.label(text="Animation (UAX)", icon='ARMATURE_DATA')
        boxAnim.prop(self, "animsEnable")
        if self.animsEnable == True:
            boxAnim.prop(self, "animsExportAll")

    #Do Export#
    def execute(self, context):
        # do your exporting here
        ExportUGX(self, context, self.filepath, self.exportMeshes, self.selectedOnly, self.animsEnable, self.animsExportAll, self.orientationBack, self.orientationRight, self.orientationUp)
        
        self.report({'INFO'}, 'Done.')
        
        return {'FINISHED'}


def menu_func_export(self, context):
    self.layout.operator(ExportUGXClass.bl_idname, text="HWDE Mesh (.ugx)")


#
#
#

# Material Section

class MatSlot(bpy.types.UIList):
    def draw_item(self, context, layout, data, item, icon, active_data, active_propname):        
        layout.prop(item, "matName", text="", emboss=False, icon='MATERIAL')
                
class MatData(bpy.types.PropertyGroup):
    matName : StringProperty(default = "Material")
    
    spec: FloatProperty(name="spec", default=500)
    specR: FloatProperty(name="specR", default=1, min=0, max=1)
    specG: FloatProperty(name="specG", default=1, min=0, max=1)
    specB: FloatProperty(name="specB", default=1, min=0, max=1)
    
    envRefl: FloatProperty(name="envRefl", default=1, min=0, max=1)
    envSharp: FloatProperty(name="envSharp", default=1, min=0, max=1)
    envFres: FloatProperty(name="envFres", default=1, min=0, max=1)
    envFresP: FloatProperty(name="envFresP", default=.5, min=0, max=1)
    
    
    path_df: StringProperty(name="path_df", default="") #diffuse
    path_nm: StringProperty(name="path_nm", default="") #normal
    path_sp: StringProperty(name="path_sp", default="") #specular
    path_op: StringProperty(name="path_op", default="") #opacity
    path_xf: StringProperty(name="path_xf", default="") #xform
    path_em: StringProperty(name="path_em", default="") #emissive
    path_ao: StringProperty(name="path_ao", default="") #ambient occlusion
    path_env: StringProperty(name="path_env", default="") #environment
    path_envm: StringProperty(name="path_envm", default="") #environmental mask
    path_emxf: StringProperty(name="path_emxf", default="") #emissive xform
    path_dt: StringProperty(name="path_dt", default="") #distortion
    path_hl: StringProperty(name="path_hl", default="") #highlight
    path_md: StringProperty(name="path_md", default="") #modulate
    
    
    uvw_df: FloatVectorProperty(name="uvw_df", default=(0.0,0.0,0.0), min=-1, max=1, size=3)
    uvw_nm: FloatVectorProperty(name="uvw_nm", default=(0.0,0.0,0.0), min=-1, max=1, size=3)
    uvw_sp: FloatVectorProperty(name="uvw_sp", default=(0.0,0.0,0.0), min=-1, max=1, size=3)
    uvw_op: FloatVectorProperty(name="uvw_op", default=(0.0,0.0,0.0), min=-1, max=1, size=3)
    uvw_xf: FloatVectorProperty(name="uvw_xf", default=(0.0,0.0,0.0), min=-1, max=1, size=3)
    uvw_em: FloatVectorProperty(name="uvw_em", default=(0.0,0.0,0.0), min=-1, max=1, size=3)
    uvw_ao: FloatVectorProperty(name="uvw_ao", default=(0.0,0.0,0.0), min=-1, max=1, size=3)
    uvw_env: FloatVectorProperty(name="uvw_env", default=(0.0,0.0,0.0), min=-1, max=1, size=3)
    uvw_envm: FloatVectorProperty(name="uvw_envm", default=(0.0,0.0,0.0), min=-1, max=1, size=3)
    uvw_emxf: FloatVectorProperty(name="uvw_emxf", default=(0.0,0.0,0.0), min=-1, max=1, size=3)
    uvw_dt: FloatVectorProperty(name="uvw_dt", default=(0.0,0.0,0.0), min=-1, max=1, size=3)
    uvw_hl: FloatVectorProperty(name="uvw_hl", default=(0.0,0.0,0.0), min=-1, max=1, size=3)
    uvw_md: FloatVectorProperty(name="uvw_md", default=(0.0,0.0,0.0), min=-1, max=1, size=3)
    
    chan_df: EnumProperty(name="", items={('0', 'Channel 0', ''),('1', 'Channel 1', ''),('2', 'Channel 2', '')}, default='0')
    chan_nm: EnumProperty(name="", items={('0', 'Channel 0', ''),('1', 'Channel 1', ''),('2', 'Channel 2', '')}, default='0')
    chan_sp: EnumProperty(name="", items={('0', 'Channel 0', ''),('1', 'Channel 1', ''),('2', 'Channel 2', '')}, default='0')
    chan_op: EnumProperty(name="", items={('0', 'Channel 0', ''),('1', 'Channel 1', ''),('2', 'Channel 2', '')}, default='0')
    chan_xf: EnumProperty(name="", items={('0', 'Channel 0', ''),('1', 'Channel 1', ''),('2', 'Channel 2', '')}, default='0')
    chan_em: EnumProperty(name="", items={('0', 'Channel 0', ''),('1', 'Channel 1', ''),('2', 'Channel 2', '')}, default='0')
    chan_ao: EnumProperty(name="", items={('0', 'Channel 0', ''),('1', 'Channel 1', ''),('2', 'Channel 2', '')}, default='0')
    chan_env: EnumProperty(name="", items={('0', 'Channel 0', ''),('1', 'Channel 1', ''),('2', 'Channel 2', '')}, default='0')
    chan_envm: EnumProperty(name="", items={('0', 'Channel 0', ''),('1', 'Channel 1', ''),('2', 'Channel 2', '')}, default='0')
    chan_emxf: EnumProperty(name="", items={('0', 'Channel 0', ''),('1', 'Channel 1', ''),('2', 'Channel 2', '')}, default='0')
    chan_dt: EnumProperty(name="", items={('0', 'Channel 0', ''),('1', 'Channel 1', ''),('2', 'Channel 2', '')}, default='0')
    chan_hl: EnumProperty(name="", items={('0', 'Channel 0', ''),('1', 'Channel 1', ''),('2', 'Channel 2', '')}, default='0')
    chan_md: EnumProperty(name="", items={('0', 'Channel 0', ''),('1', 'Channel 1', ''),('2', 'Channel 2', '')}, default='0')
    
    cFlagColorGloss : BoolProperty(name="cFlagColorGloss", default=False)
    cFlagOpacityValid : BoolProperty(name="cFlagOpacityValid", default=False)
    cFlagTwoSided : BoolProperty(name="cFlagTwoSided", default=False)
    cFlagDisableShadows : BoolProperty(name="cFlagDisableShadows", default=False)
    cFlagGlobalEnv : BoolProperty(name="cFlagGlobalEnv", default=False)
    cFlagTerrainConform : BoolProperty(name="cFlagTerrainConform", default=False)
    cFlagLocalReflection : BoolProperty(name="cFlagLocalReflection", default=False)
    cFlagDisableShadowReception : BoolProperty(name="cFlagDisableShadowReception", default=False)


def getAllUGXMaterials(self, context):
    matNames = []
    
    matNames.append(('Select', '[Select]', ""))
    
    for m in context.scene.ugxMats:
        matNames.append((m.matName, m.matName, ""))
        
    if len(matNames) == 1:
        matNames = [('NONE', 'NONE  [Add materials in \'World Properties\' menu]', "")]
        return matNames
    return matNames


class AddMaterialOperator(bpy.types.Operator):
    bl_idname = "scene.add_ugx_mat_operator"
    bl_label = "Add Material"
    
    def execute(self, context):
        bpy.context.scene.ugxMats.add()
        return {'FINISHED'}
                
class RemoveMaterialOperator(bpy.types.Operator):
    bl_idname = "scene.remove_ugx_mat_operator"
    bl_label = "Remove Material"
    
    def execute(self, context):
        
        index = bpy.context.scene.ugxMatIndex
        
        for obj in context.scene.objects:
            if obj.ugxMatIndex == index:
                obj.ugxMatIndex = 'Select'
        
        context.scene.ugxMats.remove(index)
        
        if len(context.scene.ugxMats) == 0:
            for obj in context.scene.objects:
                obj.ugxMatIndex = 'NONE'
        
        return {'FINISHED'}
        

class WorldMaterialPanel(bpy.types.Panel):
    """Creates a Panel in the Object properties window"""
    bl_label = "UGX Materials"
    bl_idname = "OBJECT_PT_allmats"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "world"



    def draw(self, context):
        layout = self.layout

        row = layout.row()
        op1 = row.operator(operator="scene.add_ugx_mat_operator", icon='ADD', emboss=True)
        op2 = row.operator(operator="scene.remove_ugx_mat_operator", icon='REMOVE', emboss=True)
        row = layout.row()
        row.template_list("MatSlot", "", context.scene, "ugxMats", context.scene, "ugxMatIndex")
       
        if len(context.scene.ugxMats) > 0:
            currentMat = context.scene.ugxMats[context.scene.ugxMatIndex]
            
            
            specBox = layout.box()
            specBox.label(text="Specular:")
            
            specBox.prop(data=currentMat, property="spec", text="Power")
            
            specRGB = specBox.split()
            specRGB.prop(data=currentMat, property="specR", text="R")
            specRGB.prop(data=currentMat, property="specG", text="G")
            specRGB.prop(data=currentMat, property="specB", text="B")
            
            
            envBox = layout.box()
            envBox.label(text="Environment:")
            
            env = envBox.split()
            env.prop(data=currentMat, property="envRefl", text="Reflectivity")
            env.prop(data=currentMat, property="envSharp", text="Sharpness")
            env = envBox.split()
            env.prop(data=currentMat, property="envFres", text="Fresnel")
            env.prop(data=currentMat, property="envFresP", text="Fresnel Power")
            
            texBox = layout.box()
            texBox.row().prop(context.scene, "textureMode", expand=True)
            
            
            if context.scene.textureMode == '0':
                texBox.prop(data=currentMat, property="path_df", text="Diffuse")
                texBox.prop(data=currentMat, property="path_nm", text="Normal")
                texBox.prop(data=currentMat, property="path_sp", text="Specular")
                texBox.prop(data=currentMat, property="path_op", text="Opacity")
                texBox.prop(data=currentMat, property="path_xf", text="XForm")
                texBox.prop(data=currentMat, property="path_em", text="Emissive")
                texBox.prop(data=currentMat, property="path_emxf", text="Emissive XF")
                texBox.prop(data=currentMat, property="path_ao", text="Occlusion")
                texBox.prop(data=currentMat, property="path_env", text="Environment")
                texBox.prop(data=currentMat, property="path_envm", text="Env Mask")
                texBox.prop(data=currentMat, property="path_dt", text="Distortion")
                texBox.prop(data=currentMat, property="path_hl", text="Highlight")
                texBox.prop(data=currentMat, property="path_md", text="Modulation")
            if context.scene.textureMode == '1':
                texBox.row().prop(data=currentMat, property="uvw_df", text="Diffuse")
                texBox.row().prop(data=currentMat, property="uvw_nm", text="Normal")
                texBox.row().prop(data=currentMat, property="uvw_sp", text="Specular")
                texBox.row().prop(data=currentMat, property="uvw_op", text="Opacity")
                texBox.row().prop(data=currentMat, property="uvw_xf", text="XForm")
                texBox.row().prop(data=currentMat, property="uvw_em", text="Emissive")
                texBox.row().prop(data=currentMat, property="uvw_emxf", text="Emissive XF")
                texBox.row().prop(data=currentMat, property="uvw_ao", text="Occlusion")
                texBox.row().prop(data=currentMat, property="uvw_env", text="Environment")
                texBox.row().prop(data=currentMat, property="uvw_envm", text="Env Mask")
                texBox.row().prop(data=currentMat, property="uvw_dt", text="Distortion")
                texBox.row().prop(data=currentMat, property="uvw_hl", text="Highlight")
                texBox.row().prop(data=currentMat, property="uvw_md", text="Modulation")
            if context.scene.textureMode == '2':
                texBox.prop(data=currentMat, property="chan_df", text="Diffuse")
                texBox.prop(data=currentMat, property="chan_nm", text="Normal")
                texBox.prop(data=currentMat, property="chan_sp", text="Specular")
                texBox.prop(data=currentMat, property="chan_op", text="Opacity")
                texBox.prop(data=currentMat, property="chan_xf", text="XForm")
                texBox.prop(data=currentMat, property="chan_em", text="Emissive")
                texBox.prop(data=currentMat, property="chan_emxf", text="Emissive XF")
                texBox.prop(data=currentMat, property="chan_ao", text="Occlusion")
                texBox.prop(data=currentMat, property="chan_env", text="Environment")
                texBox.prop(data=currentMat, property="chan_envm", text="Env Mask")
                texBox.prop(data=currentMat, property="chan_dt", text="Distortion")
                texBox.prop(data=currentMat, property="chan_hl", text="Highlight")
                texBox.prop(data=currentMat, property="chan_md", text="Modulation")


               

            
            flagBox = layout.box()
            flagBox.label(text="Flags:")
            sp1 = flagBox.split()
            sp1.prop(data=currentMat, property="cFlagColorGloss", text="ColorGloss")
            sp1.prop(data=currentMat, property="cFlagOpacityValid", text="OpacityValid")
            sp1 = flagBox.split()
            sp1.prop(data=currentMat, property="cFlagTwoSided", text="TwoSided")
            sp1.prop(data=currentMat, property="cFlagDisableShadows", text="DisableShadows")
            sp1 = flagBox.split()
            sp1.prop(data=currentMat, property="cFlagGlobalEnv", text="GlobalEnv")
            sp1.prop(data=currentMat, property="cFlagTerrainConform", text="TerrainConform")
            sp1 = flagBox.split()
            sp1.prop(data=currentMat, property="cFlagLocalReflection", text="LocalReflection")
            sp1.prop(data=currentMat, property="cFlagDisableShadowReception", text="DisableShadowReception")

class MeshMaterialPanel(bpy.types.Panel):
    """Creates a Panel in the Object properties window"""
    bl_label = "UGX Material"
    bl_idname = "OBJECT_PT_permeshmat"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "object"


    def draw(self, context):
        layout = self.layout
        
        #layout.label(text="Material Name:")
        
        if context.object.type == 'MESH':
            box = layout.box()
            box.label(text="Select Material:", icon='MATERIAL')
            box.prop(context.object, "ugxMatIndex", text="")
        
          
        #if context.object.ugxMatIndex < len(context.scene.ugxMats):
         #  layout.label(text="Material: " + context.scene.ugxMats[context.object.ugxMatIndex].matName)
        #else:
         #   layout.label(text="[INVALID MATERIAL INDEX] This material will default to 0.")
            
    

def saveMatsToXML(self, context, outDir):
    
    objs = []
    for obj in context.scene.objects:
        if obj.type == 'MESH':
            if self.selectedOnly:
                if obj in context.selected_objects:
                    objs.append(obj)
            else:
                objs.append(obj)
        
    usedMats = []
    for obj in objs:
        for curMat in context.scene.ugxMats:
            if obj.ugxMatIndex == curMat.matName:
                usedMats.append(curMat)
                
    matSet = set(usedMats)
    matIndex = {}
    
    i = 0
    for m in matSet:
        matIndex[m.matName] = i
        i+=1
        
    
    matIndexString = str(len(objs)) + "\0"
    for obj in objs:
        matIndexString += obj.name + '\0' + str(matIndex[obj.ugxMatIndex]) + '\0'
        
          
    f = open(outDir + "\\tmp.matindex", "w")
    f.write(matIndexString)
 

    rootNode = ET.Element("Materials")
    for mat in matSet:
    
        matNode = ET.SubElement(rootNode, "Material")
        
        nameValues = ET.SubElement(matNode, "NameValues")
        
        SpecPower = ET.SubElement(nameValues, "SpecPower")
        SpecPower.text = str(mat.spec)
        
        SpecColorR = ET.SubElement(nameValues, "SpecColorR")
        SpecColorR.text = str(mat.specR)
        
        SpecColorG = ET.SubElement(nameValues, "SpecColorG")
        SpecColorG.text = str(mat.specG)
        
        SpecColorB = ET.SubElement(nameValues, "SpecColorB")
        SpecColorB.text = str(mat.specB)
        
        EnvReflectivity = ET.SubElement(nameValues, "EnvReflectivity")
        EnvReflectivity.text = str(mat.envRefl)
        
        EnvSharpness = ET.SubElement(nameValues, "EnvSharpness")
        EnvSharpness.text = str(mat.envSharp)
        
        EnvFresnel = ET.SubElement(nameValues, "EnvFresnel")
        EnvFresnel.text = str(mat.envFres)
        
        EnvFresnelPower = ET.SubElement(nameValues, "EnvFresnelPower")
        EnvFresnelPower.text = str(mat.envFresP)
        
        AccessoryIndex = ET.SubElement(nameValues, "AccessoryIndex")
        AccessoryIndex.text = "0"
        
        
        #flags
        
        cFlags = 0
        cFlagColorGloss             = 1
        cFlagOpacityValid           = 2
        cFlagTwoSided               = 4
        cFlagDisableShadows         = 8
        cFlagGlobalEnv              = 16
        cFlagTerrainConform         = 32
        cFlagLocalReflection        = 64
        cFlagDisableShadowReception = 128
        
        if mat.cFlagColorGloss:
            cFlags = cFlags | cFlagColorGloss
        if mat.cFlagOpacityValid:
            cFlags = cFlags | cFlagOpacityValid
        if mat.cFlagTwoSided:
            cFlags = cFlags | cFlagTwoSided
        if mat.cFlagDisableShadows:
            cFlags = cFlags | cFlagDisableShadows
        if mat.cFlagGlobalEnv:
            cFlags = cFlags | cFlagGlobalEnv
        if mat.cFlagTerrainConform:
            cFlags = cFlags | cFlagTerrainConform
        if mat.cFlagLocalReflection:
            cFlags = cFlags | cFlagLocalReflection
        if mat.cFlagDisableShadowReception:
            cFlags = cFlags | cFlagDisableShadowReception
        
        Flags = ET.SubElement(nameValues, "Flags")
        Flags.text = str(cFlags)
        
        #####
        
        
        BlendType = ET.SubElement(nameValues, "BlendType")
        BlendType.text = "0"
        
        Opacity = ET.SubElement(nameValues, "Opacity")
        Opacity.text = "255"
        
        
        
        maps = ET.SubElement(matNode, "Maps")
        
        map = ET.SubElement(maps, "diffuse")
        uvwstr = str(mat.uvw_df[0]) + "," + str(mat.uvw_df[1]) + "," + str(mat.uvw_df[2])
        map.set("UVWVel", uvwstr)
        
        if mat.path_df != "":
            map = ET.SubElement(map, "Map")
            map.set("Channel", mat.chan_df)
            map.set("Flags", "0")
            map.set("Name", mat.path_df)
            
            
        map = ET.SubElement(maps, "normal")
        uvwstr = str(mat.uvw_nm[0]) + "," + str(mat.uvw_nm[1]) + "," + str(mat.uvw_nm[2])
        map.set("UVWVel", uvwstr)
        
        if mat.path_nm != "":
            map = ET.SubElement(map, "Map")
            map.set("Channel", mat.chan_nm)
            map.set("Flags", "0")
            map.set("Name", mat.path_nm)
            
            
        map = ET.SubElement(maps, "gloss")
        uvwstr = str(mat.uvw_sp[0]) + "," + str(mat.uvw_sp[1]) + "," + str(mat.uvw_sp[2])
        map.set("UVWVel", uvwstr)
        
        if mat.path_sp != "":
            map = ET.SubElement(map, "Map")
            map.set("Channel", mat.chan_sp)
            map.set("Flags", "0")
            map.set("Name", mat.path_sp)
            
            
        map = ET.SubElement(maps, "opacity")
        uvwstr = str(mat.uvw_op[0]) + "," + str(mat.uvw_op[1]) + "," + str(mat.uvw_op[2])
        map.set("UVWVel", uvwstr)
        
        if mat.path_op != "":
            map = ET.SubElement(map, "Map")
            map.set("Channel", mat.chan_op)
            map.set("Flags", "0")
            map.set("Name", mat.path_op)
            
            
        map = ET.SubElement(maps, "xform")
        uvwstr = str(mat.uvw_xf[0]) + "," + str(mat.uvw_xf[1]) + "," + str(mat.uvw_xf[2])
        map.set("UVWVel", uvwstr)
        
        if mat.path_xf != "":
            map = ET.SubElement(map, "Map")
            map.set("Channel", mat.chan_xf)
            map.set("Flags", "0")
            map.set("Name", mat.path_xf)
            
            
        map = ET.SubElement(maps, "emissive")
        uvwstr = str(mat.uvw_em[0]) + "," + str(mat.uvw_em[1]) + "," + str(mat.uvw_em[2])
        map.set("UVWVel", uvwstr)
        
        if mat.path_em != "":
            map = ET.SubElement(map, "Map")
            map.set("Channel", mat.chan_em)
            map.set("Flags", "0")
            map.set("Name", mat.path_em)
            
            
        map = ET.SubElement(maps, "ao")
        uvwstr = str(mat.uvw_ao[0]) + "," + str(mat.uvw_ao[1]) + "," + str(mat.uvw_ao[2])
        map.set("UVWVel", uvwstr)
        
        if mat.path_ao != "":
            map = ET.SubElement(map, "Map")
            map.set("Channel", mat.chan_ao)
            map.set("Flags", "0")
            map.set("Name", mat.path_ao)
            
            
        map = ET.SubElement(maps, "env")
        uvwstr = str(mat.uvw_env[0]) + "," + str(mat.uvw_env[1]) + "," + str(mat.uvw_env[2])
        map.set("UVWVel", uvwstr)
        
        if mat.path_env != "":
            map = ET.SubElement(map, "Map")
            map.set("Channel", mat.chan_env)
            map.set("Flags", "0")
            map.set("Name", mat.path_env)
            
            
        map = ET.SubElement(maps, "envmask")
        uvwstr = str(mat.uvw_envm[0]) + "," + str(mat.uvw_envm[1]) + "," + str(mat.uvw_envm[2])
        map.set("UVWVel", uvwstr)
        
        if mat.path_envm != "":
            map = ET.SubElement(map, "Map")
            map.set("Channel", mat.chan_envm)
            map.set("Flags", "0")
            map.set("Name", mat.path_envm)
            
            
        map = ET.SubElement(maps, "emxform")
        uvwstr = str(mat.uvw_emxf[0]) + "," + str(mat.uvw_emxf[1]) + "," + str(mat.uvw_emxf[2])
        map.set("UVWVel", uvwstr)
        
        if mat.path_emxf != "":
            map = ET.SubElement(map, "Map")
            map.set("Channel", mat.chan_emxf)
            map.set("Flags", "0")
            map.set("Name", mat.path_emxf)
            
            
        map = ET.SubElement(maps, "distortion")
        uvwstr = str(mat.uvw_dt[0]) + "," + str(mat.uvw_dt[1]) + "," + str(mat.uvw_dt[2])
        map.set("UVWVel", uvwstr)
        
        if mat.path_dt != "":
            map = ET.SubElement(map, "Map")
            map.set("Channel", mat.chan_dt)
            map.set("Flags", "0")
            map.set("Name", mat.path_dt)
            
            
        map = ET.SubElement(maps, "highlight")
        uvwstr = str(mat.uvw_hl[0]) + "," + str(mat.uvw_hl[1]) + "," + str(mat.uvw_hl[2])
        map.set("UVWVel", uvwstr)
        
        if mat.path_hl != "":
            map = ET.SubElement(map, "Map")
            map.set("Channel", mat.chan_hl)
            map.set("Flags", "0")
            map.set("Name", mat.path_hl)
            
            
        map = ET.SubElement(maps, "modulate")
        uvwstr = str(mat.uvw_md[0]) + "," + str(mat.uvw_md[1]) + "," + str(mat.uvw_md[2])
        map.set("UVWVel", uvwstr)
        
        if mat.path_md != "":
            map = ET.SubElement(map, "Map")
            map.set("Channel", mat.chan_md)
            map.set("Flags", "0")
            map.set("Name", mat.path_md)

    xmlString = ET.tostring(rootNode, encoding='unicode', method='xml')
    outStr1 = xml.dom.minidom.parseString(xmlString)
    outStr2 = outStr1.toprettyxml()
    f = open(outDir + "\\tmp.matdata", "w")
    f.write(outStr2)
    
#
#
#

# Addon Section


classes_ = (
    ExportUGXClass,
    WorldMaterialPanel,
    MatSlot,
    MatData,
    AddMaterialOperator,
    RemoveMaterialOperator,
    MeshMaterialPanel,
)

def register():    
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)

    for cls in classes_:
        bpy.utils.register_class(cls)
    
    bpy.types.Scene.ugxMats = bpy.props.CollectionProperty(type=MatData)
    bpy.types.Scene.ugxMatIndex = bpy.props.IntProperty()
    bpy.types.Scene.textureMode = bpy.props.EnumProperty(
            name="Texture Mode",
            items={('0', 'Texture', ''),('1', 'UVW', ''),('2', 'Channel', '')},
            default='0'
            )
    
    bpy.types.Object.ugxMatIndex = EnumProperty(
            name="",
            description="Select the armature UGX material.",
            items=getAllUGXMaterials,
            )
    return;


def unregister():    
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)
    
    for cls in reversed(classes_):
        bpy.utils.unregister_class(cls)
    
    del bpy.types.Scene.ugxMats
    return;