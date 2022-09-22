import { ComponentFixture, TestBed } from '@angular/core/testing';

import { GlossaireComponent } from './glossaire.component';

describe('GlossaireComponent', () => {
  let component: GlossaireComponent;
  let fixture: ComponentFixture<GlossaireComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ GlossaireComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(GlossaireComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
