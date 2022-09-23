import { Component, EventEmitter, OnInit, Output } from '@angular/core';

@Component({
  selector: 'app-references',
  templateUrl: './references.component.html',
  styleUrls: ['./references.component.css']
})
export class ReferencesComponent implements OnInit {
  links: any= [
    ["artistes", "Artistes", "emoji_people"],
    ["livres", "Livres", "menu_book"],
    ["../home", "Retour", "arrow_back"]
  ];

  sidenav_opened: boolean= false;

  @Output() hideMenuEvent = new EventEmitter<boolean>();

  constructor() { }

  ngOnInit(): void {
  }

  activate() {
    //console.log("activate ref");
    this.sidenav_opened= true;
    this.hideMenuEvent.emit(true);
  }

  deactivate() {
    //console.log("deactivate ref");
    this.sidenav_opened= false;
    this.hideMenuEvent.emit(false);
  }

}
