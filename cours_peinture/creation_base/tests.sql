--SELECT s.date, c.intitule from seance s JOIN participation p ON p.id_seance=s.id JOIN eleve e ON p.id_eleve=e.id JOIN cours c ON s.id_cours=c.id WHERE e.nom='Coulon' AND e.prenom='Ghilaine';
--SELECT p.date, p.intervenant_ou_a37, f.nom, c.intitule FROM paiement p JOIN forfait f ON p.id_forfait=f.id JOIN cours c ON f.id_cours=c.id JOIN eleve e ON p.id_eleve=e.id WHERE e.nom='Coulon' AND e.prenom='Ghilaine';

WITH participations AS (
	SELECT s.date, c.intitule from seance s JOIN participation p ON p.id_seance=s.id JOIN eleve e ON p.id_eleve=e.id JOIN cours c ON s.id_cours=c.id WHERE e.nom='Coulon' AND e.prenom='Ghilaine'
),
paiements AS (
	SELECT p.date, p.intervenant_ou_a37, f.nom, c.intitule FROM paiement p JOIN forfait f ON p.id_forfait=f.id JOIN cours c ON f.id_cours=c.id JOIN eleve e ON p.id_eleve=e.id WHERE e.nom='Coulon' AND e.prenom='Ghilaine'
)
SELECT pt.*, pa.* FROM participations pt CROSS JOIN paiements pa;
