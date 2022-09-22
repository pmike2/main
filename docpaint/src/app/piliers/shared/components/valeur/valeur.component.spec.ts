import { ComponentFixture, TestBed } from '@angular/core/testing';

import { ValeurComponent } from './valeur.component';

describe('ValeurComponent', () => {
  let component: ValeurComponent;
  let fixture: ComponentFixture<ValeurComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ ValeurComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(ValeurComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
