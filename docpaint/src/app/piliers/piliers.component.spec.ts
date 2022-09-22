import { ComponentFixture, TestBed } from '@angular/core/testing';

import { PiliersComponent } from './piliers.component';

describe('PiliersComponent', () => {
  let component: PiliersComponent;
  let fixture: ComponentFixture<PiliersComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ PiliersComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(PiliersComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
