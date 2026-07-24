Blender notes
==============

Environnement
--------------

n : properties panel
tab : passage en edit mode / object mode
ctrl + tab : choix plus large de modes ou pose mode si armature sélectionnée
o : active proportional editing avec un cercle d'influence ; permet de déformer de façon + continue; on peut ajuster la taille du cercle avec la molette
shift + tab : active / désactive snap. si désactivé 'g' permet de l'activer temporairement pour le déplacement du sommet sélectionné, sinon 'g' le désactive temporairement. Voir les différents types de snap.
touches &, é, " (1,2,3 de la barre du haut) : choix vertex / edge / face mode

Cursor
-------

shift + c : center 3d cursor
shift + rightclick : move cursor origin ; la création d'un mesh se fait sur le cursor origin


Selecting
----------

select objects + shift + click : permet de choisir l'objet actif (le dernier à avoir été clické)
x : delete sélection
a : select all
alt + a : select none
c : sélection par cercle avec rayon ajustable
cmd + right click : lasso de sélection
shift + d : duplicate selection
edit mode + p : separate by selection : met dans un nouvel objet la sélection ; separate by loose parts : met dans des objets différents tous les mesh qui sont séparés
select vertices in edit mode; cmd + g -> "assign to new group" pour créer un vertex group à partir de ces vertices
select objects + m : move to collection

Visualisation
--------------

numpad : vues
',' du numpad : zoom sur sélection
h : hide selection ; alt + h unhide ; shift + h : hide all except selection
shift + z : wireframe mode / solid mode ; en wireframe la sélection se fait sur tous les sommets / edges / faces intersectant
alt + z : x-ray mode, pareil que shift + z, mais on reste shading solid
viewport overlays / turn on wireframe : pour voir les aretes

Modeling
----------

**Faire les modèles avec -Y comme direction où le perso regarde, sinon les symétries des bones ne fonctionnent pas**

shift + a : fenetre de création de formes
f : création de face ou edge
cmd + r : loopcut ; right-click : reste au centre ; wheel : ajoute des cuts
cmd + b : bevel (biseau) edge (wheel permet de gérer le nombre de edges)
cmd + shift + b : bevel vertex (wheel permet de gérer le nombre de sommets)
cmd + x : dissolve vertices
j : join vertices
k : knife : permet d'ajouter des edges
select edge ou 2 edges opposés + f : fill -> crée une face

g : translation
r : rotation
s : scale ; 0 pour aplatir
translation, rot, scale : (x , y , ou z) pour spécifier un axe ; shift + (x , y ou z) pour ignorer un axe
alt + s : shrink / fatten ; effet différent de scale
shift quand on fait une rotation ou scale ou translation permet de faire petit à petit
translation d'un vertex : g + g (une 2ème fois) permet de déplacer un vertex le long d'une arête
alt + g : clear location (alt r : clear rotation , alt s : clear scale)

e : extrude
i : inset face
alt + left click : select loop
cmd + e : menu -> edge slide : fait glisser un edge de part et d'autre de sa face
cmd + j : join selected objects

Pour afficher l'orientation des faces, en mode edit , overlays, 'Face orientation'
Si une face est rouge c'est mauvais, la sélectionner, puis menu Mesh / Normals / Flip

cmd + a en object mode -> apply all transforms : permet d'appliquer les transformations d'un objet à ses vertices et de reinit les transfos de l'objet

icone barre du haut en forme de cible permet de changer le pivot point (3d cursor par ex). Le défaut est median point.
individual origins permet de faire des rotations autour de chaque centre d'objet

icone en haut à droite : auto merge vertices : fusionne les vertices qui se chevauchent


Rigging
--------

# extrusion
e : extrude bone
Quand on extrude à partir du bone entier sélectionné ou de son tip, le bone extruded sera l'enfant du bone
Quand on extrude à partir de la base d'un bone sans parent, le bone extruded n'aura pas de parent non plus
Dans l'onglet bone, section relations, on peut déconnecter un bone, gérer le parent et d'autres trucs.

Data / viewport display / in front pour afficher armature devant le reste
shift + d : duplicate bones
ctrl + p : make parent
alt + p : unmake parent (disconnect)
nommer les jambes bras gauche avec suffix .L, puis click droit symmetrize : crée les .R
object mode ; select model ; click droit armature : parent / armature defform / automatic weights -> crée des vertex groups et assigne des poids automatiquement
on peut aussi créer les vertex group à la main mais il faut que les noms des vertex group correspondent aux noms des bones
et il faut que le mesh ait un modifier de type armature
Menu principal à droite de armature : show names -> affiche le nom des bones

# weight painting
1. object mode, select armature
2. shift + click select object
3. ctrl + tab -> weight paint mode
barre du haut / weights / assign automatic from bones : permet de repartir de ce qu'on a fait avant avec automatic weights
brush tool options / auto normalize : pour que la somme des weights fasse tjrs 1
brush tool symmetry : utile pour bras / jambes
f : change brush size
dans menu principal à droite : Vertex Groups : on peut switcher entre les bones pour peindre leur influence sur l'objet sélectionné

# rigid rigging
fonctionne uniquement quand on a des objets distincts
1. object mode, select un objet (par ex la tête)
2. shift click armature
3. ctrl + tab -> pose mode
4. select bone
5. ctrl + p : set parent to bone -> l'objet est maintenant attaché au bone


Deformers
----------

**On peut appliquer définitivement les modifiers avec onglet modifier fleche du bas pour afficher le menu déroulant du modifier et faire apply, mais c'est mieux de ne pas le faire pour conserver plus de possibilités.**

**select multiple objects + cmd l : link data / copy modifiers : permet de copier les modifiers de l'objet actif vers les autres**

# Mirror
Option clipping permet de n'avoir qu'un seul sommet à la frontière
Déplacer les sommets d'un objet en edit mode (pas en object mode car sinon on déplace également le pt de ref de l'objet)
Pour faire un objet symmétrique se placer en vue Front (numpad 1), scale 0.5 selon x, puis translation 0.5 selon x, puis application deformer mirror
on peut créer un objet empty / plain axes et s'en servir comme d'un centre pour le déformer symmétrie

# Shrinkwrap
permet de projeter des sommets d'un objet sur un autre objet, on peut également choisir un vertex group

# Lattice
1. ajout lattice ; onglet lattice permet de gérer avec u, v, w la résolution du lattice
2. sur un objet ajout derfomer lattice que l'on lie avec le lattice créé.
3. maintenant quand on déforme le lattice, l'objet est aussi déformé


Materials
----------

on peut mettre plusieurs matériaux pour un objet, puis en edit mode sélectionner des faces et click Assign dans onglet Material
pour assigner ce mat aux faces

select multiple objects + cmd l : link materials -> tous les objets ont le même matérial que l'objet actif

# Texturing
onglet material, click point color -> image texture
ouvrir un UV editor en haut à droite
passer en mode edit
passer en material preview pour voir la texture sur le modèle
u : unwrap selection ; si on sélectionne une face toute la texture sera appliquée à la face
ctrl + e menu ; mark seam : sépare les objets le long de la sélection d'edges

# Normal map
1. ouvrir shader editor
2. ajouter un noeud normal map (add / displacement / normal map)
3. ajouter un noeud texture (add / texture / image texture)
4. passer le color space de la texture en 'Non color'
5. charger la texture normal (générée par gimp avec filtres / génériques / carte normale)
6. connecter la sortie color de la texture normale à l'entrée color de la normal map et la sortie normal de la normal map à l'entrée normal de la BSDF
7. jouer avec l'intensité


Rigging
--------

dupliquer fenetre : dope sheet window + timeline
select all bones avec 'a' puis 'i' pour insérer un keyframe
copy pose puis paste x-flipped pose permet de copier une version miroir (d'ou l'interet de faire regarder le perso vers Y négatif en bind pose)
t : permet de choisir le mode d'interpolation
pour faire une marche : dupliquer puis inverser toutes les poses
g, s : translation, scale des keyframes sélectionnés
a : select all, puis x : supprime tous les keyframes
markers : utiles si on veut mettre plusieurs anims dans un .blend (marcher, courir, ...)
Marker / add marker pour ajouter un marker, puis on peut le renommer en le sélectionnant puis rename marker

Reset :
a : select all bones
alt + r : reset all rotations
alt + g : reset all translations
alt + s : reset all scales

Références
-----------

https://polyhaven.com : site où récupérer des textures et des hdri (sert à éclairer des scènes 3d)
