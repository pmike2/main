import json

import bpy


# aller voir :
# https://blender.stackexchange.com/questions/44637/how-can-i-manually-calculate-bpy-types-posebone-matrix-using-blenders-python-ap/121495#121495


def matrix2list__(m):
    return [
        m[0][0], m[0][1], m[0][2], m[0][3],
        m[1][0], m[1][1], m[1][2], m[1][3],
        m[2][0], m[2][1], m[2][2], m[2][3],
        m[3][0], m[3][1], m[3][2], m[3][3],
    ]


def matrix2list___(m):
    return [
        m[0][0], m[1][0], m[2][0], m[3][0],
        m[0][1], m[1][1], m[2][1], m[3][1],
        m[0][2], m[1][2], m[2][2], m[3][2],
        m[0][3], m[1][3], m[2][3], m[3][3],
    ]

def matrix2list(m):
    return [
        list(m[0]), list(m[1]), list(m[2]), list(m[3])
    ]


def matrix_world(bone_name):
    armature_ob = bpy.data.objects["Armature"]
    bone = armature_ob.pose.bones[bone_name]
    return armature_ob.matrix_world @ bone.matrix


def matrix_world__(bone_name):
    armature_ob = bpy.data.objects["Armature"]
    
    local = armature_ob.data.bones[bone_name].matrix_local
    basis = armature_ob.pose.bones[bone_name].matrix_basis

    parent = armature_ob.pose.bones[bone_name].parent
    if parent == None:
        return  local @ basis
    else:
        parent_local = armature_ob.data.bones[parent.name].matrix_local
        return matrix_world(parent.name) @ (parent_local.inverted() @ local) @ basis
    

def write_some_data(context, filepath, use_some_setting):
    
    data = {"bones" : {}, "objects" : [], "actions" : {}}
    
    #l_bones = bpy.data.armatures[0].bones.items()
    l_bones = bpy.context.object.pose.bones

    for bone in l_bones:
        #bone_name = bone[0]
        #bone_obj = bone[1]
        
        bone_name = bone.name
        parent = bone.parent
        if parent is None:
            parent_name = None
        else:
            parent_name = parent.name
        
        #matrix = bone_obj.matrix
        #matrix_list = []
        #for i in range(3):
        #   matrix_list.append([matrix[i][0], matrix[i][1], matrix[i][2]])
        
        data["bones"][bone_name] = {
            "parent" : parent_name,
            "matrix_local" : matrix2list(bone.bone.matrix_local)
        }
        
    markers = {}
    for marker in bpy.context.scene.timeline_markers:
        marker_name = marker.name
        marker_frame = marker.frame
        
        action = marker_name.split("_")[0]
        end_or_start = marker_name.split("_")[1]
        
        if end_or_start == "start":
            markers[action] = {"start" : marker_frame}
        elif end_or_start == "end":
            markers[action]["end"] = marker_frame
    
    for action, marker_dic in markers.items():
    #for f in range(bpy.context.scene.frame_start, bpy.context.scene.frame_end + 1):
        
        data["actions"][action] = []
        
        for f in range(marker_dic["start"], marker_dic["end"]):
            bpy.context.scene.frame_set(f)
        
            dic_frame = {}
            for bone in l_bones:
                #dic_frame[bone.name] = matrix2list(bone.matrix)
                dic_frame[bone.name] = {
                    "matrix" : matrix2list(bone.matrix),
                    "matrix_basis" : matrix2list(bone.matrix_basis),
                    "matrix_world" : matrix2list(matrix_world(bone.name)),
                    "location" : list(bone.location),
                    "rotation_quaternion" : list(bone.rotation_quaternion)
                }
        
            data["actions"][action].append(dic_frame)


    l_objects = bpy.data.objects.items()
    for obj_ in l_objects:
        obj_name = obj_[0]
        obj = obj_[1]
        if obj_name == "Armature":
            continue
        parent_bone_name = obj.parent_bone
        
        data["objects"].append({"name" : obj_name, "bone" : parent_bone_name})
    
    
    
    with open(filepath, 'w') as f:
        json.dump(data, f, indent=4)
    
    return {'FINISHED'}


# ExportHelper is a helper class, defines filename and
# invoke() function which calls the file selector.
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty, BoolProperty, EnumProperty
from bpy.types import Operator


class ExportSomeData(Operator, ExportHelper):
    """This appears in the tooltip of the operator and in the generated docs"""
    bl_idname = "export_test.some_data"  # Important since its how bpy.ops.import_test.some_data is constructed.
    bl_label = "Export Some Data"

    # ExportHelper mix-in class uses this.
    filename_ext = ".json"

    filter_glob: StringProperty(
        default="*.json",
        options={'HIDDEN'},
        maxlen=255,  # Max internal buffer length, longer would be clamped.
    )

    # List of operator properties, the attributes will be assigned
    # to the class instance from the operator settings before calling.
    use_setting: BoolProperty(
        name="Example Boolean",
        description="Example Tooltip",
        default=True,
    )

    type: EnumProperty(
        name="Example Enum",
        description="Choose between two items",
        items=(
            ('OPT_A', "First Option", "Description one"),
            ('OPT_B', "Second Option", "Description two"),
        ),
        default='OPT_A',
    )

    def execute(self, context):
        return write_some_data(context, self.filepath, self.use_setting)


# Only needed if you want to add into a dynamic menu
def menu_func_export(self, context):
    self.layout.operator(ExportSomeData.bl_idname, text="Text Export Operator")


# Register and add to the "file selector" menu (required to use F3 search "Text Export Operator" for quick access).
def register():
    bpy.utils.register_class(ExportSomeData)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(ExportSomeData)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)


if __name__ == "__main__":
    register()

    # Test call.
    bpy.ops.export_test.some_data('INVOKE_DEFAULT')
