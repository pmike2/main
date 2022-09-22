import { ComponentFixture, TestBed } from '@angular/core/testing';

import { PinceauComponent } from './pinceau.component';

describe('PinceauComponent', () => {
  let component: PinceauComponent;
  let fixture: ComponentFixture<PinceauComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ PinceauComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(PinceauComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
