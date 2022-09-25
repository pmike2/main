import { Component, EventEmitter, OnInit, Output } from '@angular/core';
import { Router } from '@angular/router';

@Component({
  selector: 'app-materiaux',
  templateUrl: './materiaux.component.html',
  styleUrls: ['./materiaux.component.css']
})
export class MateriauxComponent implements OnInit {
  links: any= [
    ["support", "Support", "content_paste"],
    ["medium", "Médium", "liquor"],
    ["pinceau", "Pinceaux", "brush"],
    ["couleur-mat", "Couleurs", "invert_colors"],
    ["palette", "Palette", "palette"],
    ["nettoyage", "Nettoyage", "wash"],
    ["../home", "Retour", "arrow_back"]
  ];

  sidenav_opened: boolean= false;

  @Output() hideMenuEvent = new EventEmitter<boolean>();

  constructor(private router: Router) { }

  ngOnInit(): void {
    
  }

  activate() {
    //console.log("activate materiaux");
    this.sidenav_opened= true;
    this.hideMenuEvent.emit(true);
    this.router.navigate(["materiaux/support"]);
  }

  deactivate() {
    //console.log("deactivate materiaux");
    this.sidenav_opened= false;
    this.hideMenuEvent.emit(false);
  }

}
