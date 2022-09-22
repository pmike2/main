import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';

import {MenuComponent} from './menu/menu.component';
import {IntroComponent} from './intro/intro.component';
import {ReferencesComponent} from './references/references.component';
import { GlossaireComponent } from './glossaire/glossaire.component';
import { DeroulementComponent } from './deroulement/deroulement.component';
import { GeneralComponent } from './general/general.component';
import { InstallationComponent } from './installation/installation.component';
import { MateriauxComponent } from './materiaux/materiaux.component';
import { PiliersComponent } from './piliers/piliers.component';

const routes: Routes = [
  {path : 'intro', component : IntroComponent},
  {path : 'references', component : ReferencesComponent},
  {path : 'glossaire', component : GlossaireComponent},
  {path : 'deroulement', component : DeroulementComponent},
  {path : 'general', component : GeneralComponent},
  {path : 'installation', component : InstallationComponent},
  {path : 'materiaux', component : MateriauxComponent},
  {path : 'piliers', component : PiliersComponent},
];

@NgModule({
  imports: [RouterModule.forRoot(routes)],
  exports: [RouterModule]
})
export class AppRoutingModule { }
