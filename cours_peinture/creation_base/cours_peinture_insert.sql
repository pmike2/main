-- modele -----------------------------------------------------------------------------
INSERT INTO modele (nom, prenom, mail, phone) VALUES
('?', 'Aurore', '?', '0767842135'),
('?', 'Nina', '?', '0695032802'),
('?', 'Marcabrune', '?', '0761170421'),
('?', 'Louise', '?', '0682623655'),
('?', 'Lucine', '?', '0687535841'),
('?', 'Ludovic', '?', '0658218294'),
('?', 'Rebecca', '?', '0640210090');



-- modele -----------------------------------------------------------------------------
INSERT INTO eleve (nom, prenom, mail, phone) VALUES
('Vallat', 'Jean-Marc', 'jmvfamilly@gmail.com', '0685909604'),
('Fournier', 'Vincent', 'vf.fournier@free.fr', '0601848897'),
('?', 'Laurence', '?', '?'),
('Nadau', 'Eric', 'ernadau@gmail.com', '0642653365'),
('Ibos', 'Catherine', 'katy.ibos@gmail.com', '0642572005'),
('Miralles', 'Maryse', '?', '?'),
('Amiand', 'Maryam', 'maryam.l.amiand@gmail.com', '0661497518'),
('Wojszvzyk', 'Clémentine', 'clementine.wojszvzyk@gmail.com', '0627450553'),
('Coulon', 'Ghilaine', 'ghilaine.coulon@orange.fr', '0660483826'),
('Haurie', 'Sandrine', 'sandrine.haurie@free.fr', '0615351460'),
('Haurie', 'SandrineFille', '?', '?'),
('Baudrimont', 'Denis', 'denis.baudrimont@gmail.com', '?'),
('Sieng', 'Frederic', 'frederic.sieng@gmail.com', '?'),
('KY SOTH', 'Danielle', 'd.kysoth@orange.fr', '0683286335'),
('Vandermarcq', 'Olivier', 'vandermarcq.olivier@orange.fr', '?');


-- intervenant --------------------------------------------------------------------------
INSERT INTO intervenant (nom, prenom, mail, phone) VALUES
('Beau', 'Pierre-Michael', 'piermibeau@gmail.com', '0687054513');


-- cours ------------------------------------------------------------------------------
INSERT INTO cours(id_intervenant, intitule, pourcentage_a37, duree, annee)
SELECT id, 'Portrait alla prima', 30.0, '2 hours'::interval, '2026-01-01' FROM intervenant WHERE nom = 'Beau';

INSERT INTO cours(id_intervenant, intitule, pourcentage_a37, duree, annee)
SELECT id, 'Portrait alla prima', 30.0, '2 hours'::interval, '2027-01-01' FROM intervenant WHERE nom = 'Beau';

INSERT INTO cours(id_intervenant, intitule, pourcentage_a37, duree, annee)
SELECT id, 'Stage Portrait', 30.0, '3 hours'::interval, '2026-08-29' FROM intervenant WHERE nom = 'Beau';


-- forfait ----------------------------------------------------------------------------
WITH id_cours_2026 AS (
	SELECT id FROM cours WHERE intitule = 'Portrait alla prima' AND annee = '2026-01-01'
),
forfaits_alla_prima AS (
	SELECT * FROM (VALUES ('decouverte', 1, 20.0), ('1_cours', 1, 30.0), ('4_cours', 4, 112.0), ('10_cours', 10, 250.0), ('trimestre', 15, 300.0), ('annee', 40, 650.0)) AS t(nom, n_seances, cout_total)
)
INSERT INTO forfait(id_cours, nom, n_seances, cout_total)
SELECT idc.id, fap.nom, fap.n_seances, fap.cout_total FROM id_cours_2026 AS idc CROSS JOIN forfaits_alla_prima AS fap;

WITH id_cours_2027 AS (
	SELECT id FROM cours WHERE intitule = 'Portrait alla prima' AND annee = '2027-01-01'
),
forfaits_alla_prima AS (
	SELECT * FROM (VALUES ('decouverte', 1, 20.0), ('1_cours', 1, 30.0), ('4_cours', 4, 112.0), ('10_cours', 10, 250.0), ('trimestre', 15, 300.0), ('annee', 40, 650.0)) AS t(nom, n_seances, cout_total)
)
INSERT INTO forfait(id_cours, nom, n_seances, cout_total)
SELECT idc.id, fap.nom, fap.n_seances, fap.cout_total FROM id_cours_2027 AS idc CROSS JOIN forfaits_alla_prima AS fap;

WITH id_stage_2026 AS (
	SELECT id FROM cours WHERE intitule = 'Stage Portrait' AND annee = '2026-08-29'
),
forfaits_stage AS (
	SELECT * FROM (VALUES ('unique', 1, 50.0)) AS t(nom, n_seances, cout_total)
)
INSERT INTO forfait(id_cours, nom, n_seances, cout_total)
SELECT idc.id, fap.nom, fap.n_seances, fap.cout_total FROM id_stage_2026 AS idc CROSS JOIN forfaits_stage AS fap;


-- seance ----------------------------------------------------------------------------
WITH id_cours_2026 AS (
	SELECT id FROM cours WHERE intitule = 'Portrait alla prima' AND annee = '2026-01-01'
),
date_cours_2026 AS (
	SELECT d FROM (VALUES ('2026-03-12'), ('2026-03-19'), ('2026-03-26'), ('2026-04-02'), ('2026-04-09'), 
	('2026-04-16'), ('2026-05-07'), ('2026-05-21'), ('2026-05-28'), ('2026-06-04'), ('2026-06-11'), ('2026-06-18')) AS t(d)
)
INSERT INTO seance(id_cours, date)
SELECT idc.id, dc.d::DATE FROM id_cours_2026 AS idc CROSS JOIN date_cours_2026 AS dc;

WITH id_cours_2027 AS (
	SELECT id FROM cours WHERE intitule = 'Portrait alla prima' AND annee = '2027-01-01'
),
date_cours_2027 AS (
	SELECT d FROM (VALUES ('2026-09-10'), ('2026-09-17')) AS t(d)
)
INSERT INTO seance(id_cours, date)
SELECT idc.id, dc.d::DATE FROM id_cours_2027 AS idc CROSS JOIN date_cours_2027 AS dc;


-- participation ------------------------------------------------------------------------
-- voir .py

-- paiement ------------------------------------------------------------------------
-- voir .py

-- paiement_modele ----------------------------------------------------------------------
-- voir .py
