import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';

import {MatListModule} from '@angular/material/list';
import {MatIconModule} from '@angular/material/icon';
import {MatSidenavModule} from '@angular/material/sidenav';
import {MatExpansionModule} from '@angular/material/expansion';
import {MatCardModule} from '@angular/material/card';
import {MatTabsModule} from '@angular/material/tabs';

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
import { HomeComponent } from './home/home.component';
import { CompositionComponent } from './piliers/shared/components/composition/composition.component';


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
    CouleurMatComponent,
    HomeComponent,
    CompositionComponent
  ],
  imports: [
    BrowserModule,
    AppRoutingModule,
    BrowserAnimationsModule,
    MatListModule,
    MatIconModule,
    MatSidenavModule,
    MatExpansionModule,
    MatCardModule,
    MatTabsModule
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
