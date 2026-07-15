"""
Script d'export d'une animation Blender.

2 modes (weight ou rigid) à choisir via la checkbox au moment de l'export.
JSON résultant à utiliser dans animated_obj.h

voir : https://blender.stackexchange.com/questions/44637/how-can-i-manually-calculate-bpy-types-posebone-matrix-using-blenders-python-ap/121495
"""

import json

import bpy
# ExportHelper is a helper class, defines filename and
# invoke() function which calls the file selector.
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty, BoolProperty, EnumProperty
from bpy.types import Operator


# nom par défaut de l'armature
# TODO : gérer plusieurs armatures ou le fait que l'armature ne s'appelle pas Armature ?
ARMATURE_DEFAULT_NAME = "Armature"


# renvoie les valeurs de la matrice m sous forme de liste de rows
# du coup attention lors de l'utilisation dans opengl qui est column-major
def matrix2list(m):
    return [
        list(m[0]), list(m[1]), list(m[2]), list(m[3])
    ]


def export_weight(filepath):
    """Export de l'animation avec les poids."""
    data = {"armature" : [], "bones" : {}, "actions" : {}, "fps" : bpy.context.scene.render.fps}

    # on sélectionne l'armature, sinon erreur au moment de récupérer les bones
    bpy.context.view_layer.objects.active = bpy.data.objects[ARMATURE_DEFAULT_NAME]

    # matrice de transfo de l'armature ( = mat4(1.0) si l'origine de l'armature est en (0,0,0))
    data["armature"] = matrix2list(bpy.data.objects[ARMATURE_DEFAULT_NAME].matrix_world)
    
    l_bones = bpy.context.object.pose.bones
    
    # pour chaque bone
    for bone in l_bones:
        # récupération du parent
        bone_name = bone.name
        parent = bone.parent
        if parent is None:
            parent_name = None
        else:
            parent_name = parent.name
        
        # pour chaque objet
        weights = {}
        for obj in bpy.data.objects:
            if obj.name == ARMATURE_DEFAULT_NAME:
                continue
            
            weights[obj.name] = {}
            
            # les sommets de l'objet
            obj_verts = obj.data.vertices
            
            # dans le projet Blender il faut associer par un nom identique un vertex group et un bone
            if bone.name not in obj.vertex_groups:
                continue
            
            # indice du vertex group affecté par ce bone
            gidx = obj.vertex_groups[bone.name].index
            # sommets appartenant à ce vertex group
            bone_verts = [v for v in obj_verts if gidx in [g.group for g in v.groups]]

            # pour chaque sommet, assignation du poids au sein de l'objet
            # attention on ne peut pas identifier un sommet uniquement par son index car chaque objet l'initialise à 0
            # ce qui est unique c'est le couple (objet, v.index)
            for v in bone_verts:
                for g in v.groups:
                    if g.group == gidx: 
                        weights[obj.name][v.index] = g.weight
                        break
        
        # set data for bone bone_name
        data["bones"][bone_name] = {
            "parent" : parent_name,
            "matrix_local" : matrix2list(bone.bone.matrix_local),
            "weights" : weights
        }
    
    # lecture des markers
    markers = {}
    for marker in bpy.context.scene.timeline_markers:
        # nom du marker
        marker_name = marker.name

        # index frame du marker dans la timeline
        marker_frame = marker.frame
        
        # attention : il faut nommer les marqueurs d'une action xxx_start / xxx_end
        action = marker_name.split("_")[0]
        end_or_start = marker_name.split("_")[1]
        
        if end_or_start == "start":
            markers[action] = {"start" : marker_frame}
        elif end_or_start == "end":
            markers[action]["end"] = marker_frame
    
    # actions
    for action, marker_dic in markers.items():
        data["actions"][action] = []
        
        # pour chaque frame
        for f in range(marker_dic["start"], marker_dic["end"]):
            # on se positionne sur ce frame
            bpy.context.scene.frame_set(f)
        
            dic_frame = {}
            # on note pour chaque os sa matrice de transfo dans ce frame
            for bone in l_bones:
                dic_frame[bone.name] = {
                    "matrix_basis" : matrix2list(bone.matrix_basis),
                }
        
            data["actions"][action].append(dic_frame)


    with open(filepath, 'w') as f:
        json.dump(data, f, indent=4)
    
    return {'FINISHED'}


def export_rigid(filepath):
    """Export de l'animation en rigide."""
    data = {"armature" : [], "bones" : {}, "actions" : {}, "objects" : [], "fps" : bpy.context.scene.render.fps}

    bpy.context.view_layer.objects.active = bpy.data.objects[ARMATURE_DEFAULT_NAME]

    data["armature"] = matrix2list(bpy.data.objects[ARMATURE_DEFAULT_NAME].matrix_world)
    
    l_bones = bpy.context.object.pose.bones

    for bone in l_bones:
        bone_name = bone.name
        parent = bone.parent
        if parent is None:
            parent_name = None
        else:
            parent_name = parent.name
        
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
        data["actions"][action] = []
        
        for f in range(marker_dic["start"], marker_dic["end"]):
            bpy.context.scene.frame_set(f)
        
            dic_frame = {}
            for bone in l_bones:
                dic_frame[bone.name] = {
                    "matrix_basis" : matrix2list(bone.matrix_basis),
                }
        
            data["actions"][action].append(dic_frame)

    # dans ce mode on cherche à associer un objet et un bone
    # c'est quelque chose qu'il faut avoir fait dans le projet Blender
    l_objects = bpy.data.objects
    for obj_name, obj in l_objects.items():
        if obj_name == ARMATURE_DEFAULT_NAME:
            continue
        parent_bone_name = obj.parent_bone
        
        data["objects"].append({"name" : obj_name, "bone" : parent_bone_name})
    
    with open(filepath, 'w') as f:
        json.dump(data, f, indent=4)
    
    return {'FINISHED'}


class ExportAnimation(Operator, ExportHelper):
    """Export animation."""
    bl_idname = "export_animation.export_animation"
    bl_label = "Export Some Data"

    # ExportHelper mix-in class uses this.
    filename_ext = ".json"

    filter_glob: StringProperty(
        default="*.json",
        options={'HIDDEN'},
        maxlen=255,
    )

    mode: EnumProperty(
        name="mode",
        description="mode d'animation",
        items=(
            ('weight', "weight", "animation par poids"),
            ('rigid', "rigid", "animation rigide"),
        ),
        default='weight',
    )

    def execute(self, context):
        if self.mode == "weight":
            return export_weight(self.filepath)
        elif self.mode == "rigid":
            return export_rigid(self.filepath)


# Register and add to the "file selector" menu (required to use F3 search "Text Export Operator" for quick access).
def register():
    bpy.utils.register_class(ExportAnimation)


def unregister():
    bpy.utils.unregister_class(ExportAnimation)


if __name__ == "__main__":
    register()

    # Test call.
    bpy.ops.export_animation.export_animation('INVOKE_DEFAULT')
