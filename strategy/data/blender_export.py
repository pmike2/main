import json

import bpy


# https://docs.blender.org/api/current/bpy.types.Bone.html#bpy.types.Bone


def write_some_data(context, filepath, use_some_setting):
    
    data = {"bones" : [], "objects" : []}
    
    l_bones = bpy.data.armatures[0].bones.items()

    for bone in l_bones:
        bone_name = bone[0]
        bone_obj = bone[1]
        
        parent = bone_obj.parent
        if parent is None:
            parent_name = None
        else:
            parent_name = parent.name
        matrix = bone_obj.matrix
        matrix_list = []
        for i in range(3):
            matrix_list.append([matrix[i][0], matrix[i][1], matrix[i][2]])
        
        data["bones"].append({"name" : bone_name, "parent" : parent_name, "matrix" : matrix_list})


    l_objects = bpy.data.objects.items()
    for obj_ in l_objects:
        obj_name = obj_[0]
        obj = obj_[1]
        if obj_name == "Armature":
            continue
        parent_bone_name = obj.parent_bone
        
        data["objects"].append({"name" : obj_name, "bone" : parent_bone_name})
    
    scene = bpy.data.scenes[0]
    
    
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
