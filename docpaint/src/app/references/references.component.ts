import { Component, EventEmitter, OnInit, Output } from '@angular/core';
import { Router } from '@angular/router';

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

  constructor(private router: Router) { }

  ngOnInit(): void {
  }

  activate() {
    //console.log("activate ref");
    this.sidenav_opened= true;
    this.hideMenuEvent.emit(true);
    this.router.navigate(["references/artistes"]);
  }

  deactivate() {
    //console.log("deactivate ref");
    this.sidenav_opened= false;
    this.hideMenuEvent.emit(false);
  }

}
