Blender notes
==============

Environnement
--------------

n : properties panel
numpad : vues
',' du numpad : zoom sur sélection
tab : passage en edit mode / object mode
ctrl + tab : choix plus large de modes ou pose mode si armature sélectionnée
shift + z : wireframe mode / solid mode ; en wireframe la sélection se fait sur tous les sommets / edges / faces intersectant
o : active proportional editing avec un cercle d'influence ; permet de déformer de façon + continue; on peut ajuster la taille du cercle avec la molette
shift + tab : active / désactive snap. si désactivé 'g' permet de l'activer temporairement pour le déplacement du sommet sélectionné, sinon 'g' le désactive temporairement. Voir les différents types de snap.
touches &, é, " (1,2,3 de la barre du haut) : choix vertex / edge / face mode

Cursor
-------

shift + c : center 3d cursor


Selecting
----------

x : delete sélection
a : select all
alt + a : select none
c : sélection par cercle avec rayon ajustable
shift + d : duplicate selection
edit mode + p : separate by selection : met dans un nouvel objet la sélection ; separate by loose parts : met dans des objets différents tous les mesh qui sont séparés
select vertices in edit mode; cmd + g -> "assign to new group" pour créer un vertex group à partir de ces vertices

Visualisation
--------------

h : hide selection ; alt + h unhide ; shift + h : hide all except selection
viewport overlays / turn on wireframe : pour voir les aretes

Modelling
----------

**Faire les modèles avec -Y comme direction où le perso regarde, sinon les symétries des bones ne fonctionnent pas**
shift + a : fenetre de création de formes
f : création de face ou edge
cmd + r : loopcut ; right-click : reste au centre ; wheel : ajoute des cuts
cmd + b : bevel (biseau) edge (wheel permet de gérer le nombre de edges)
cmd + shift + b : bevel vertex (wheel permet de gérer le nombre de sommets)

g : translation
r : rotation
s : scale ; 0 pour aplatir
translation, rot, scale : (x , y , ou z) pour spécifier un axe ; shift + (x , y ou z) pour ignorer un axe
alt + s : shrink / fatten ; effet différent de scale

e : extrude
i : inset face
alt + left click : select loop
cmd + e : menu -> edge slide : fait glisser un edge de part et d'autre de sa face
cmd + j : merge selected objects

Deformers
----------

Mirror : option clipping permet de n'avoir qu'un seul sommet à la frontière


Texturing
----------

onglet material, click point color -> image texture
ouvrir un UV editor en haut à droite
passer en mode edit
passer en material preview pour voir la texture sur le modèle
u : unwrap selection ; si on séléctionne une face toute la texture sera appliquée à la face
ctrl + e menu ; mark seam : sépare les objets le long de la sélection d'edges
alt + z : show material color


Rigging
--------

dupliquer fenetre : passer en dope sheet window
select all bones avec 'a' puis 'i' pour insérer un keyframe
copy pose puis paste x-flipped pose permet de copier une version miroir (d'ou l'interet de faire regarder le perso vers Y négatif en bind pose)
t : permet de choisir le mode d'interpolation
pour faire une marche : dupliquer puis inverser toutes les poses
g, s : translation, scale des keyframes sélectionnés

Reset :
a : select all bones
alt + r : reset all rotations
alt + g : reset all translations
alt + s : reset all scales

