# aller voir :
# https://antongerdelan.net/opengl/blend_shapes.html

import json

import bpy


# suffixes des markers
ACTION_TYPE_START = "start"
ACTION_TYPE_END = "end"
ACTION_TYPE_LOOPSTART = "loopstart"
ACTION_TYPE_LOOPEND = "loopend"


def get_markers():
    """Lecture des markers."""
    markers = {}
    for marker in bpy.context.scene.timeline_markers:
        # nom du marker
        marker_name = marker.name

        # index frame du marker dans la timeline
        marker_frame = marker.frame
        
        # attention : il faut nommer les marqueurs d'une action xxx_start / xxx_end / xxx_loopstart / xxx_loopend
        action = marker_name.split("_")[0]
        action_type = marker_name.split("_")[1]
        assert action_type in (ACTION_TYPE_START, ACTION_TYPE_END, ACTION_TYPE_LOOPSTART, ACTION_TYPE_LOOPEND), f"action_type {action_type} non supporté"

        if action not in markers.keys():
            markers[action] = dict.fromkeys([ACTION_TYPE_START, ACTION_TYPE_END, ACTION_TYPE_LOOPSTART, ACTION_TYPE_LOOPEND])

        markers[action][action_type] = marker_frame

    return markers


def test():
    #obj = bpy.data.objects[0]
    #shape_key = obj.active_shape_key

    markers = get_markers()
    
    data = {}

    for key in bpy.data.shape_keys:
        obj_name = key.user.name
        
        data[obj_name] = {"shape_keys" : {}, "actions" : {}}
    
        for shape_key in key.key_blocks:
            shape_key_name = shape_key.name
            #if shape_key_name == "Basis":
            #    continue
            
            data[obj_name]["shape_keys"][shape_key_name] = []
        
            for pt in shape_key.points:
                pos = pt.co
                data[obj_name]["shape_keys"][shape_key_name].append([pos[0], pos[1], pos[2]])

        for action, marker_dic in markers.items():
            data[obj_name]["actions"][action] = []
            # pour chaque frame
            for f in range(marker_dic[ACTION_TYPE_START], marker_dic[ACTION_TYPE_END] + 1):
                # on se positionne sur ce frame
                bpy.context.scene.frame_set(f)
                
                dic_frame =  {}

                for shape_key in key.key_blocks:
                    shape_key_name = shape_key.name
                    if shape_key_name == "Basis":
                        continue
                    
                    value = shape_key.value
                    dic_frame[shape_key_name] = value
                    
                data[obj_name]["actions"][action].append(dic_frame)

    filepath = "/Users/pmbeau2/test.json"
    with open(filepath, 'w') as f:
        json.dump(data, f, indent=4)


if __name__ == "__main__":
    test()
    