import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'app-menu',
  templateUrl: './menu.component.html',
  styleUrls: ['./menu.component.css']
})
export class MenuComponent implements OnInit {
  links: any= [
    ["/home", "Home", "home"],
    ["/intro", "Intro", "info"],
    ["materiaux", "Matériaux", "brush"],
    ["installation", "Installation", "build"],
    ["general", "Conseils généraux", "lightbulb"],
    ["piliers", "Les 4 piliers", "foundation"],
    ["deroulement", "Déroulement d'un portrait", "fast_forward"],
    ["references", "Références", "menu_book"],
    ["glossaire", "Glossaire", "list"]
  ];

  sidenav_opened: boolean= true;

  constructor() { }

  ngOnInit(): void {
  }
}
