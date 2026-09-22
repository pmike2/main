--SELECT s.date, c.intitule from seance s JOIN participation p ON p.id_seance=s.id JOIN eleve e ON p.id_eleve=e.id JOIN cours c ON s.id_cours=c.id WHERE e.nom='Coulon' AND e.prenom='Ghilaine';
--SELECT p.date, p.intervenant_ou_a37, f.nom, c.intitule FROM paiement p JOIN forfait f ON p.id_forfait=f.id JOIN cours c ON f.id_cours=c.id JOIN eleve e ON p.id_eleve=e.id WHERE e.nom='Coulon' AND e.prenom='Ghilaine';

--SELECT SUM(f.cout_total) FROM forfait f JOIN paiement p ON p.id_forfait=f.id JOIN cours c ON f.id_cours=c.id WHERE c.annee = '2026-01-01' AND c.intitule = 'Portrait alla prima' AND p.intervenant_ou_a37 = 'A37';

SELECT SUM(p.montant) FROM paiement_modele p JOIN seance s ON p.id_seance=s.id JOIN cours c ON s.id_cours=c.id WHERE c.annee = '2026-01-01' AND c.intitule = 'Portrait alla prima' AND p.intervenant_ou_a37 = 'A37';
