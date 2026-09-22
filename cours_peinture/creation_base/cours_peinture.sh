# lancement de la base
pg_ctl -D /Volumes/Data/perso/paint_db start

# sauvegarde
current_date=`date +"%Y-%m-%d"`
pg_dump --clean a37 > /Volumes/DESIR/PMB/perso/cours_peinture_db_${current_date}.sql
