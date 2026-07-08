import json

import bpy



def matrix2list(m):
    return [
        list(m[0]), list(m[1]), list(m[2]), list(m[3])
    ]


def write_some_data(context, filepath, use_some_setting):
    
    data = {"bones" : {}, "objects" : [], "actions" : {}}
    
    l_bones = bpy.context.object.pose.bones
    
    obj = bpy.data.objects['Cube']
    obj_verts = obj.data.vertices
    
    for bone in l_bones:
        
        bone_name = bone.name
        parent = bone.parent
        if parent is None:
            parent_name = None
        else:
            parent_name = parent.name
        
        gidx = obj.vertex_groups[bone.name].index
        bone_verts = [v for v in obj_verts if gidx in [g.group for g in v.groups]]
        weights = {}
        for v in bone_verts:
            for g in v.groups:
                if g.group == gidx: 
                    weights[v.index] = g.weight
                    break
        
        data["bones"][bone_name] = {
            "parent" : parent_name,
            "matrix_local" : matrix2list(bone.bone.matrix_local),
            "weights" : weights
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
        
        data["actions"][action] = []
        
        for f in range(marker_dic["start"], marker_dic["end"]):
            bpy.context.scene.frame_set(f)
        
            dic_frame = {}
            for bone in l_bones:
                dic_frame[bone.name] = {
                    #"matrix" : matrix2list(bone.matrix),
                    "matrix_basis" : matrix2list(bone.matrix_basis),
                    #"matrix_world" : matrix2list(matrix_world(bone.name)),
                    #"location" : list(bone.location),
                    #"rotation_quaternion" : list(bone.rotation_quaternion)
                }
        
            data["actions"][action].append(dic_frame)


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
#def menu_func_export(self, context):
#    self.layout.operator(ExportSomeData.bl_idname, text="Text Export Operator")


# Register and add to the "file selector" menu (required to use F3 search "Text Export Operator" for quick access).
def register():
    bpy.utils.register_class(ExportSomeData)
    #bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(ExportSomeData)
    #bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)


if __name__ == "__main__":
    register()

    # Test call.
    bpy.ops.export_test.some_data('INVOKE_DEFAULT')
