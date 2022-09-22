import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';

import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { IntroComponent } from './intro/intro.component';
import { MenuComponent } from './menu/menu.component';
import { ReferencesComponent } from './references/references.component';
import { MateriauxComponent } from './materiaux/materiaux.component';
import { InstallationComponent } from './installation/installation.component';
import { GeneralComponent } from './general/general.component';
import { NettoyageComponent } from './materiaux/shared/components/nettoyage/nettoyage.component';
import { MediumComponent } from './materiaux/shared/components/medium/medium.component';
import { PaletteComponent } from './materiaux/shared/components/palette/palette.component';
import { PinceauComponent } from './materiaux/shared/components/pinceau/pinceau.component';
import { SupportComponent } from './materiaux/shared/components/support/support.component';
import { ArtisteComponent } from './references/shared/components/artiste/artiste.component';
import { LivreComponent } from './references/shared/components/livre/livre.component';
import { PiliersComponent } from './piliers/piliers.component';
import { CouleurComponent } from './piliers/shared/components/couleur/couleur.component';
import { ValeurComponent } from './piliers/shared/components/valeur/valeur.component';
import { DessinComponent } from './piliers/shared/components/dessin/dessin.component';
import { BordComponent } from './piliers/shared/components/bord/bord.component';
import { GlossaireComponent } from './glossaire/glossaire.component';
import { DeroulementComponent } from './deroulement/deroulement.component';
import { CouleurMatComponent } from './materiaux/shared/components/couleur-mat/couleur-mat.component';

@NgModule({
  declarations: [
    AppComponent,
    IntroComponent,
    MenuComponent,
    ReferencesComponent,
    MateriauxComponent,
    InstallationComponent,
    GeneralComponent,
    NettoyageComponent,
    MediumComponent,
    CouleurComponent,
    PaletteComponent,
    PinceauComponent,
    SupportComponent,
    ArtisteComponent,
    LivreComponent,
    PiliersComponent,
    ValeurComponent,
    DessinComponent,
    BordComponent,
    GlossaireComponent,
    DeroulementComponent,
    CouleurMatComponent
  ],
  imports: [
    BrowserModule,
    AppRoutingModule
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
