# lancement de la base
pg_ctl -D /Volumes/Data/perso/paint_db start

# sauvegarde
current_date=`date +"%Y-%m-%d"`
pg_dump --clean a37 > /Volumes/DESIR/PMB/perso/cours_peinture_db_${current_date}.sql

# activation env conda
conda activate test_streamlit

# lancement streamlit
streamlit run /Volumes/Data/perso/dev/main/cours_peinture/cours_peinture_app/cours_peinture_app.py

# url à consulter
http://localhost:8501/
