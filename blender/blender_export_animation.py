"""
Script d'export d'une animation Blender.

2 modes (weight ou rigid) à choisir via la checkbox au moment de l'export.
JSON résultant à utiliser dans animated_obj.h

voir : https://blender.stackexchange.com/questions/44637/how-can-i-manually-calculate-bpy-types-posebone-matrix-using-blenders-python-ap/121495
"""

import os

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

# nom du modifier de type armature par défaut
ARMATURE_DEFAULT_MODIFIER_NAME = "Armature"


def matrix2list(m):
    """Renvoie les valeurs de la matrice m sous forme de liste de rows.
    Du coup attention lors de l'utilisation dans opengl qui est column-major."""
    return [
        list(m[0]), list(m[1]), list(m[2]), list(m[3])
    ]


def get_armature_matrix():
    """Matrice de transfo de l'armature ( = mat4(1.0) si l'origine de l'armature est en (0,0,0))."""
    return matrix2list(bpy.data.objects[ARMATURE_DEFAULT_NAME].matrix_world)


def get_markers():
    """Lecture des markers."""
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
    
    return markers


def get_actions():
    """Récupère les actions."""
    markers = get_markers()
    actions = {}

    for action, marker_dic in markers.items():
        actions[action] = []
        
        # pour chaque frame
        for f in range(marker_dic["start"], marker_dic["end"] + 1):
            # on se positionne sur ce frame
            bpy.context.scene.frame_set(f)
        
            dic_frame = {}
            # on note pour chaque os sa matrice de transfo dans ce frame
            for bone in bpy.context.object.pose.bones:
                dic_frame[bone.name] = {
                    "matrix_basis" : matrix2list(bone.matrix_basis),
                }
        
            actions[action].append(dic_frame)
    
    return actions


def get_rigid_objects_parent_bones():
    """Dans ce mode on cherche à associer un objet et un bone.
    c'est quelque chose qu'il faut avoir fait dans le projet Blender"""
    objects = []
    for obj in bpy.data.objects:
        if obj.name == ARMATURE_DEFAULT_NAME:
            continue
        parent_bone_name = obj.parent_bone
        
        objects.append({"name" : obj.name, "bone" : parent_bone_name})

    return objects


def get_rigid_bones():
    """Bones en mode rigide."""
    bones = {}
    for bone in bpy.context.object.pose.bones:
        bone_name = bone.name
        parent = bone.parent
        if parent is None:
            parent_name = None
        else:
            parent_name = parent.name
        
        bones[bone_name] = {
            "parent" : parent_name,
            "matrix_local" : matrix2list(bone.bone.matrix_local)
        }
    return bones


def get_weight_bones():
    """Bones en mode weight."""
    bones = {}
    for bone in bpy.context.object.pose.bones:
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
        bones[bone_name] = {
            "parent" : parent_name,
            "matrix_local" : matrix2list(bone.bone.matrix_local),
            "weights" : weights
        }
    
    return bones


def export_weight(filepath):
    """Export de l'animation avec les poids."""
    data = {
        "armature" : get_armature_matrix(),
        "bones" : get_weight_bones(),
        "actions" : get_actions(),
        "fps" : bpy.context.scene.render.fps
    }

    with open(filepath, 'w') as f:
        json.dump(data, f, indent=4)


def export_rigid(filepath):
    """Export de l'animation en rigide."""
    data = {
        "armature" : get_armature_matrix(),
        "bones" : get_rigid_bones(),
        "actions" : get_actions(),
        "objects" : get_rigid_objects_parent_bones(),
        "fps" : bpy.context.scene.render.fps
    }

    with open(filepath, 'w') as f:
        json.dump(data, f, indent=4)


def apply_modifiers():
    """Application des modifiers. (sauf Armature)"""
    for obj in bpy.data.objects:
        if obj.name == ARMATURE_DEFAULT_NAME:
            continue
        
        bpy.context.view_layer.objects.active = obj

        for modifier in obj.modifiers:
            # il ne faut pas appliquer le modifier de type Armature car on veut le obj dans son état de repos
            if modifier.name == ARMATURE_DEFAULT_MODIFIER_NAME:
                continue
            
            bpy.ops.object.modifier_apply(modifier=modifier.name)


def export_obj(obj_path):
    """Export .obj."""
    bpy.ops.wm.obj_export(
        filepath=obj_path,
        check_existing=True,
        filter_blender=False,
        filter_backup=False,
        filter_image=False,
        filter_movie=False,
        filter_python=False,
        filter_font=False,
        filter_sound=False,
        filter_text=False,
        filter_archive=False,
        filter_btx=False,
        filter_alembic=False,
        filter_usd=False,
        filter_obj=False,
        filter_volume=False,
        filter_folder=True,
        filter_blenlib=False,
        filemode=8,
        display_type='DEFAULT',
        sort_method='DEFAULT',
        export_animation=False,
        start_frame=-2147483648,
        end_frame=2147483647,
        # par défaut Blender est en Y forward, Z up ; on garde ça; mais il faut faire des persos qui regardent vers -Y
        # d'ailleurs si on charge la tete de singe dans Blender elle regarde bien vers -Y
        forward_axis='Y',
        up_axis='Z',
        global_scale=1.0,
        # on veut l'objet dans son état de repos, donc pas d'application de modifier Armature
        apply_modifiers=False,
        apply_transform=False,
        export_eval_mode='DAG_EVAL_VIEWPORT',
        export_selected_objects=False,
        export_uv=True,
        # on veut les normales
        export_normals=True,
        export_colors=False,
        # on veut le .mat
        export_materials=True,
        export_pbr_extensions=False,
        path_mode='AUTO',
        # nécessaire, obj_parser.h ne lit que les triangles
        export_triangulated_mesh=True,
        export_curves_as_nurbs=False,
        export_object_groups=False,
        export_material_groups=False,
        export_vertex_groups=False,
        export_smooth_groups=False,
        smooth_group_bitflags=False,
        filter_glob='*.obj;*.mtl'
    )


def export_animation(json_path, mode):
    """Fonction principale."""
    # sauvegarde du projet afin de revenir à un état propre à la fin
    bpy.ops.wm.save_mainfile()
    
    # on passe en mode objet
    bpy.ops.object.mode_set(mode="OBJECT")
    
    # application des modifiers (mirror, shrinkwrap, ...) mais PAS armature
    apply_modifiers()
    
    # export .obj
    obj_path = os.path.splitext(json_path)[0] + ".obj"
    export_obj(obj_path)

    # on sélectionne l'armature, sinon erreur au moment de récupérer les bones
    bpy.context.view_layer.objects.active = bpy.data.objects[ARMATURE_DEFAULT_NAME]
    
    # export animation
    if mode == "weight":
        export_weight(json_path)
    elif mode == "rigid":
        export_rigid(json_path)
    
    # un peu bourrin mais permet de revenir à l'état du projet au moment de la sauvegarde donc avant application des modifiers
    bpy.ops.wm.revert_mainfile()
    
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
        return export_animation(self.filepath, self.mode)


# Only needed if you want to add into a dynamic menu
def menu_func_export(self, context):
    self.layout.operator(ExportAnimation.bl_idname, text="PM : Export animation")

# Register and add to the "file selector" menu (required to use F3 search "Text Export Operator" for quick access).
def register():
    bpy.utils.register_class(ExportAnimation)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(ExportAnimation)
    #bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)


if __name__ == "__main__":
    register()

    # Test call.
    bpy.ops.export_animation.export_animation('INVOKE_DEFAULT')
