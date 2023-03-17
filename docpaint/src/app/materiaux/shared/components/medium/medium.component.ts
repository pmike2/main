import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'app-medium',
  templateUrl: './medium.component.html',
  styleUrls: ['./medium.component.css', '../../css/materiaux.css']
})
export class MediumComponent implements OnInit {

  mediums= [
    {
      "nom" : "mix huile de lin / térébenthine 50/50", "description" : "Mélange d'huile de lin et de térébenthine",
      "contenu" : "Ce mélange 50 / 50 permet d'avoir un médium qui sèche assez vite tout en étant pas trop maigre. Il existe plusieurs types d'huile.",
      "avantages" : ["pas cher", "on peut ajuster le ratio afin d'avoir un médium plus ou moins maigre"],
      "inconvenients" : ["vapeurs toxiques de térébenthine"],
      "img" : "assets/images/terebenthine.jpg"
    },
    {
      "nom" : "gel", "description" : "gels de toute sorte",
      "contenu" : "Il existe plusieurs types de médiums sous forme de gel. Liquin est brillant. Le gel medium Sennelier est non nocif et ne modifie pas le temps de séchage",
      "avantages" : ["moins de vapeurs toxiques", "peut se fixer sur une palette"],
      "inconvenients" : ["plus cher"],
      "img" : "assets/images/gel.jpg"
    }
  ];

  constructor() { }

  ngOnInit(): void {
  }

}
