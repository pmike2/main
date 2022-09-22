import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DeroulementComponent } from './deroulement.component';

describe('DeroulementComponent', () => {
  let component: DeroulementComponent;
  let fixture: ComponentFixture<DeroulementComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ DeroulementComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(DeroulementComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
