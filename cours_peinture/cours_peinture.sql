-- types -----------------------------------------------------------------------------
CREATE TYPE intervenant_ou_a37_type AS ENUM ('INTERVENANT', 'A37');


-- modele -----------------------------------------------------------------------------
CREATE TABLE modele (
	id INTEGER,
	nom VARCHAR,
	prenom VARCHAR,
	mail VARCHAR,
	phone VARCHAR
);

ALTER TABLE modele
	ALTER id ADD GENERATED ALWAYS AS IDENTITY,
	ALTER id SET NOT NULL,
	ALTER nom SET NOT NULL,
	ALTER prenom SET NOT NULL,
	ADD CONSTRAINT modele_pkey PRIMARY KEY(id),
	ADD CONSTRAINT modele_nom_prenom_key UNIQUE(nom, prenom);


-- intervenant --------------------------------------------------------------------------
CREATE TABLE intervenant (
	id INTEGER,
	nom VARCHAR,
	prenom VARCHAR,
	mail VARCHAR,
	phone VARCHAR
);

ALTER TABLE intervenant
	ALTER id ADD GENERATED ALWAYS AS IDENTITY,
	ALTER id SET NOT NULL,
	ALTER nom SET NOT NULL,
	ALTER prenom SET NOT NULL,
	ADD CONSTRAINT intervenant_pkey PRIMARY KEY(id),
	ADD CONSTRAINT intervenant_nom_prenom_key UNIQUE(nom, prenom);


-- eleve ----------------------------------------------------------------------------
CREATE TABLE eleve (
	id INTEGER,
	nom VARCHAR,
	prenom VARCHAR,
	mail VARCHAR,
	phone VARCHAR
);

ALTER TABLE eleve
	ALTER id ADD GENERATED ALWAYS AS IDENTITY,
	ALTER id SET NOT NULL,
	ALTER nom SET NOT NULL,
	ALTER prenom SET NOT NULL,
	ADD CONSTRAINT eleve_pkey PRIMARY KEY(id),
	ADD CONSTRAINT eleve_nom_prenom_key UNIQUE(nom, prenom);


-- cours -----------------------------------------------------------------------------
CREATE TABLE cours (
	id INTEGER,
	id_intervenant INTEGER,
	intitule VARCHAR,
	pourcentage_a37 NUMERIC(4, 2),
	duree INTERVAL,
	annee DATE
);

ALTER TABLE cours
	ALTER id ADD GENERATED ALWAYS AS IDENTITY,
	ALTER id SET NOT NULL,
	ALTER id_intervenant SET NOT NULL,
	ALTER intitule SET NOT NULL,
	ALTER pourcentage_a37 SET NOT NULL,
	ALTER duree SET NOT NULL,
	ALTER annee SET NOT NULL,
	ADD CONSTRAINT cours_pkey PRIMARY KEY(id),
	ADD CONSTRAINT cours_intitule_anne_key UNIQUE(intitule, annee),
	ADD CONSTRAINT cours_intervenant_fk FOREIGN KEY(id_intervenant) REFERENCES intervenant(id) ON DELETE CASCADE;


-- forfait ----------------------------------------------------------------------------
CREATE TABLE forfait (
	id INTEGER,
	id_cours INTEGER,
	nom VARCHAR,
	n_seances INTEGER,
	cout_total NUMERIC(5, 2),
	cout_par_seance NUMERIC(5, 2) GENERATED ALWAYS AS (cout_total / n_seances) STORED
);

ALTER TABLE forfait
	ALTER id ADD GENERATED ALWAYS AS IDENTITY,
	ALTER id SET NOT NULL,
	ALTER id_cours SET NOT NULL,
	ALTER nom SET NOT NULL,
	ALTER n_seances SET NOT NULL,
	ALTER cout_total SET NOT NULL,
	ALTER cout_par_seance SET NOT NULL,
	ADD CONSTRAINT forfait_pkey PRIMARY KEY(id),
	ADD CONSTRAINT forfait_id_cours_nom_key UNIQUE(id_cours, nom),
	ADD CONSTRAINT forfait_cours_fk FOREIGN KEY(id_cours) REFERENCES cours(id) ON DELETE CASCADE;


-- seance ----------------------------------------------------------------------------
CREATE TABLE seance (
	id INTEGER,
	id_cours INTEGER,
	date TIMESTAMP
);

ALTER TABLE seance
	ALTER id ADD GENERATED ALWAYS AS IDENTITY,
	ALTER id SET NOT NULL,
	ALTER id_cours SET NOT NULL,
	ALTER date SET NOT NULL,
	ADD CONSTRAINT seance_pkey PRIMARY KEY(id),
	ADD CONSTRAINT seance_id_cours_date_key UNIQUE(id_cours, date),
	ADD CONSTRAINT seance_cours_fk FOREIGN KEY(id_cours) REFERENCES cours(id) ON DELETE CASCADE;


-- participation ----------------------------------------------------------------------------
CREATE TABLE participation (
	id INTEGER,
	id_eleve INTEGER,
	id_seance INTEGER
);

ALTER TABLE participation
	ALTER id ADD GENERATED ALWAYS AS IDENTITY,
	ALTER id SET NOT NULL,
	ALTER id_eleve SET NOT NULL,
	ALTER id_seance SET NOT NULL,
	ADD CONSTRAINT participation_pkey PRIMARY KEY(id),
	ADD CONSTRAINT participation_id_eleve_id_seance_key UNIQUE(id_eleve, id_seance),
	ADD CONSTRAINT participation_eleve_fk FOREIGN KEY(id_eleve) REFERENCES eleve(id) ON DELETE CASCADE,
	ADD CONSTRAINT participation_seance_fk FOREIGN KEY(id_seance) REFERENCES seance(id) ON DELETE CASCADE;


-- paiement ----------------------------------------------------------------------------
CREATE TABLE paiement (
	id INTEGER,
	id_eleve INTEGER,
	id_forfait INTEGER,
	date DATE,
	intervenant_ou_a37 intervenant_ou_a37_type
);

ALTER TABLE paiement
	ALTER id ADD GENERATED ALWAYS AS IDENTITY,
	ALTER id SET NOT NULL,
	ALTER id_eleve SET NOT NULL,
	ALTER id_forfait SET NOT NULL,
	ALTER date SET NOT NULL,
	ALTER intervenant_ou_a37 SET NOT NULL,
	ADD CONSTRAINT paiement_id_eleve_id_forfait_date_key UNIQUE(id_eleve, id_forfait, date),
	ADD CONSTRAINT paiement_eleve_fk FOREIGN KEY(id_eleve) REFERENCES eleve(id) ON DELETE CASCADE,
	ADD CONSTRAINT paiement_forfait_fk FOREIGN KEY(id_forfait) REFERENCES forfait(id) ON DELETE CASCADE;


-- paiement_modele -------------------------------------------------------------------------
CREATE TABLE paiement_modele (
	id INTEGER,
	id_modele INTEGER,
	id_seance INTEGER,
	intervenant_ou_a37 intervenant_ou_a37_type
);

ALTER TABLE paiement_modele
	ALTER id ADD GENERATED ALWAYS AS IDENTITY,
	ALTER id SET NOT NULL,
	ALTER id_modele SET NOT NULL,
	ALTER id_seance SET NOT NULL,
	ALTER intervenant_ou_a37 SET NOT NULL,
	ADD CONSTRAINT paiement_modele_id_modele_id_seance_key UNIQUE(id_modele, id_seance),
	ADD CONSTRAINT paiement_modele_modele_fk FOREIGN KEY(id_modele) REFERENCES modele(id) ON DELETE CASCADE,
	ADD CONSTRAINT paiement_modele_seance_fk FOREIGN KEY(id_seance) REFERENCES seance(id) ON DELETE CASCADE;
