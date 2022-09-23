import { Component, EventEmitter, OnInit, Output } from '@angular/core';

@Component({
  selector: 'app-piliers',
  templateUrl: './piliers.component.html',
  styleUrls: ['./piliers.component.css']
})
export class PiliersComponent implements OnInit {
  links: any= [
    ["dessin", "Dessin", ""],
    ["valeur", "Valeur", ""],
    ["couleur", "Couleur", ""],
    ["bord", "Bord", ""],
    ["back", "Retour", "arrow_back"]
  ];

  sidenav_opened: boolean= false;

  @Output() hideMenuEvent = new EventEmitter<boolean>();

  constructor() { }

  ngOnInit(): void {
  }

  activate() {
    //console.log("activate piliers");
    this.sidenav_opened= true;
    this.hideMenuEvent.emit(true);
  }

  deactivate() {
    //console.log("deactivate piliers");
    this.sidenav_opened= false;
    this.hideMenuEvent.emit(false);
  }

}
