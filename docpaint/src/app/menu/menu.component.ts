import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'app-menu',
  templateUrl: './menu.component.html',
  styleUrls: ['./menu.component.css']
})
export class MenuComponent implements OnInit {
  links: any= [
    ["home", "Home", "home"],
    ["intro", "Intro", "info"],
    ["materiaux", "Matériaux", "brush"],
    ["installation", "Installation", "build"],
    ["general", "Conseils généraux", "lightbulb"],
    ["piliers", "Les 4 piliers", "foundation"],
    ["deroulement", "Déroulement d'un portrait", "fast_forward"],
    ["references", "Références", "menu_book"],
    ["glossaire", "Glossaire", "list"],
  ];

  sidenav_opened: boolean= true;

  constructor() { }

  ngOnInit(): void {
  }

  onActivate(elementRef: any) {
    //console.log(elementRef);
    if (elementRef.hideMenuEvent!== undefined) {
      elementRef.hideMenuEvent.subscribe((event: any) => {
        //console.log(event);
        this.hide_menu(event);
      });
    }
    
    if (elementRef.activate!== undefined) {
      elementRef.activate();
    }
  }

  onDeactivate(elementRef: any) {
    if (elementRef.deactivate!== undefined) {
      elementRef.deactivate();
    }
  }

  hide_menu(event : any) {
    //console.log("hide menu : "+ event);
    this.sidenav_opened= !event;
  }
}
