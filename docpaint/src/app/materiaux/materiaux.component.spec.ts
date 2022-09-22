import { ComponentFixture, TestBed } from '@angular/core/testing';

import { MateriauxComponent } from './materiaux.component';

describe('MateriauxComponent', () => {
  let component: MateriauxComponent;
  let fixture: ComponentFixture<MateriauxComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ MateriauxComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(MateriauxComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
