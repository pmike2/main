import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';

import {IntroComponent} from './intro/intro.component';
import {ReferencesComponent} from './references/references.component';
import { GlossaireComponent } from './glossaire/glossaire.component';
import { DeroulementComponent } from './deroulement/deroulement.component';
import { GeneralComponent } from './general/general.component';
import { InstallationComponent } from './installation/installation.component';
import { MateriauxComponent } from './materiaux/materiaux.component';
import { PiliersComponent } from './piliers/piliers.component';
import { HomeComponent } from './home/home.component';
import { MediumComponent } from './materiaux/shared/components/medium/medium.component';
import { CouleurMatComponent } from './materiaux/shared/components/couleur-mat/couleur-mat.component';
import { NettoyageComponent } from './materiaux/shared/components/nettoyage/nettoyage.component';
import { PaletteComponent } from './materiaux/shared/components/palette/palette.component';
import { PinceauComponent } from './materiaux/shared/components/pinceau/pinceau.component';
import { SupportComponent } from './materiaux/shared/components/support/support.component';
import { BordComponent } from './piliers/shared/components/bord/bord.component';
import { CouleurComponent } from './piliers/shared/components/couleur/couleur.component';
import { DessinComponent } from './piliers/shared/components/dessin/dessin.component';
import { ValeurComponent } from './piliers/shared/components/valeur/valeur.component';
import { LivreComponent } from './references/shared/components/livre/livre.component';
import { ArtisteComponent } from './references/shared/components/artiste/artiste.component';


// l'ordre est important
const routes: Routes = [
  {path : 'intro', component : IntroComponent},
  {path : 'references', component : ReferencesComponent, children : [
    {path : 'livres', component : LivreComponent},
    {path : 'artistes', component : ArtisteComponent},
  ]},
  {path : 'glossaire', component : GlossaireComponent},
  {path : 'deroulement', component : DeroulementComponent},
  {path : 'general', component : GeneralComponent},
  {path : 'installation', component : InstallationComponent},
  {path : 'materiaux', component : MateriauxComponent, children : [
    {path : 'couleur-mat', component : CouleurMatComponent},
    {path : 'medium', component : MediumComponent},
    {path : 'nettoyage', component : NettoyageComponent},
    {path : 'palette', component : PaletteComponent},
    {path : 'pinceau', component : PinceauComponent},
    {path : 'support', component : SupportComponent},
  ]},
  {path : 'piliers', component : PiliersComponent, children : [
    {path : 'bord', component : BordComponent},
    {path : 'couleur', component : CouleurComponent},
    {path : 'dessin', component : DessinComponent},
    {path : 'valeur', component : ValeurComponent},
  ]},
  { path : 'home', component: HomeComponent },
  { path : '**', component: HomeComponent },
];

@NgModule({
  imports: [RouterModule.forRoot(routes)],
  exports: [RouterModule]
})
export class AppRoutingModule { }
