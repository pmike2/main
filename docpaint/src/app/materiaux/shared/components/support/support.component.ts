import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'app-support',
  templateUrl: './support.component.html',
  styleUrls: ['./support.component.css', '../../css/materiaux.css']
})
export class SupportComponent implements OnInit {
  supports: any = [
    {
      "nom" : "Canevas", "description" : "toile tendue sur un chassis",
      "contenu" : "C'est le support préféré lors de peintures grands formats, car allège et permet la découpe afin de le transporter plus aisément. La taille peut être très variable; adapter sa sélection de pinceaux à la taille du support.",
      "avantages" : ["supporte les grands formats", "transportable modulo découpe"],
      "inconvenients" : ["peut se détendre"],
      "img" : "assets/images/canevas.jpg"},
    {
      "nom" : "Feuille", "description" : "bloc de feuilles spécifique peinture à l'huile",
      "contenu" : "Plusieurs marques proposent du papier spécifique à la peinture à l'huile. Les tailles vont de à .",
      "avantages" : ["léger"],
      "inconvenients" : ["besoin d'un support solide en plus"],
      "img" : "assets/images/feuille.png"},
    {
      "nom" : "Planche bois", "description" : "comme la Joconde !",
      "contenu" : "planche de bois en hêtre ? ou ?",
      "avantages" : ["pas de déformation"],
      "inconvenients" : ["lourd", "demande une préparation"],
      "img" : "assets/images/bois.jpg"
    },
    {
      "nom" : "Aluminium", "description" : "panneau d'alu",
      "contenu" : "Feuille d'aluminium collée à un panneau, permet de créer un rendu très lumineux",
      "avantages" : ["un rendu unique"],
      "inconvenients" : ["cher", "demande une préparation"],
      "img" : "assets/images/aluminium.jpg"
    },
  ];

  constructor() { }

  ngOnInit(): void {
  }

}
