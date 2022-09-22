import { ComponentFixture, TestBed } from '@angular/core/testing';

import { CouleurMatComponent } from './couleur-mat.component';

describe('CouleurMatComponent', () => {
  let component: CouleurMatComponent;
  let fixture: ComponentFixture<CouleurMatComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ CouleurMatComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(CouleurMatComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
