(
w = Window.new("GUI Introduction", Rect(200,200,255,100));
b = Button.new(w,Rect(10,0,80,30)).states_([["Hide"],["Show"]]);
l = Slider.new(w,Rect(95,0,150,30));
c = CompositeView.new(w,Rect(20,35,100,60));
StaticText.new(c,Rect(0,0,80,30)).string_("Hello");
StaticText.new(c,Rect(20,30,80,30)).string_("World!");
b.action = { c.visible = b.value.asBoolean.not };
l.action = { c.bounds = Rect( l.value * 150 + 20, 35, 100, 100 ) };
w.front;
)

(
w = Window.new("GUI Introduction", Rect(200,200,200,200));
TextField.new(w,Rect(0,0,200,30)).resize_(2);
Slider.new(w,Rect(0,30,30,170)).resize_(4);
TextView.new(w,Rect(30,30,170,170)).resize_(5);
w.front;
)

(
w = Window.new("GUI Introduction", Rect(200,200,320,320)).front;
// notice that FlowLayout refers to w.view, which is the container view
// automatically created with the window and occupying its entire space
w.view.decorator = FlowLayout(w.view.bounds);
14.do{ Slider(w, 150@20) };
)

(
w = Window.new("GUI Introduction").layout_(
    VLayout(
        HLayout( Button(), TextField(), Button() ),
        TextView()
    )
).front;
)

(
w = Window("GUI Introduction").background_(Color.rand).front;
b = Button(w, Rect(10,10,100,30)).states_([
    ["One",Color.rand,Color.rand],
    ["Two",Color.rand,Color.rand],
    ["Three",Color.rand,Color.rand]
]);
l = ListView.new(w, Rect(10,50,200,100))
    .items_(["One","Two","Three"])
    .colors_([Color.rand,Color.rand,Color.rand])
    .hiliteColor_(Color.blue)
    .selectedStringColor_(Color.white);
r = Slider(w, Rect(10, 160, 200, 20))
    .knobColor_(Color.rand)
    .background_(Color.rand);
)


(
x = QPalette.auto(Color.red(0.8), Color.red(0.5));
y = QPalette.auto(Color.cyan(1.4), Color.cyan(1.8));
p = QtGUI.palette;
QtGUI.palette = x;
w = Window.new("GUI Introduction").front;
w.onClose = {QtGUI.palette = p};
Button.new(w, Rect(10,10,100,30)).states_([
    ["Red", Color.black, Color.grey(0.7)],
    ["Cyan"]
]).action_({ |b| QtGUI.palette = if(b.value == 0){x}{y} });
ListView.new(w, Rect(10,50,200,100))
    .items_(["One","Two","Three"])
    .colors_([Color.grey(0.4),Color.grey(0.5),Color.grey(0.6)]);
Slider(w, Rect(10, 160, 200, 20));
RangeSlider(w, Rect(10, 190, 200, 20));
)


(
w = Window.new("GUI Introduction",Rect(200,200,200,70)).front;
a = [Font.defaultMonoFace, Font.defaultSansFace, Font.defaultSerifFace];
b = Button.new(w,Rect(10,10,180,50))
    .states_([["Monospace"],["Sans serif"],["Serif"]])
    .font_(a[0])
    .action_({|b| b.font = a[b.value]});
)

(
~makeWindow = { var w;
    w = Window.new("Evader",Rect(500.rand + 100, 500.rand + 100, 200,50)).front;
    Button.new(w,Rect(10,10,180,30)).states_([["Evade"]]).action_(~makeWindow);
};
~makeWindow.value;
)


(
w = Window.new(bounds:Rect(200,200,200,50)).front;
b = Button.new(w,Rect(10,10,80,30)).states_([["Off"],["On"]]);
t = StaticText(w,Rect(100,10,90,30)).string_("Button released");
b.mouseDownAction = { t.string = "Button pressed" };
b.mouseUpAction = { t.string = "Button released" };
)

(
w = Window.new(bounds:Rect(200,200,200,100)).front;
x = TextField(w,Rect(10,10,80,30));
y = TextField(w,Rect(110,10,80,30));
t = StaticText(w,Rect(10,40,180,50));
~reset = {t.background = Color.red};
x.keyDownAction = {t.background = Color.green};
x.keyUpAction = ~reset;
y.keyDownAction = {t.background = Color.blue};
y.keyUpAction = ~reset;
~reset.value;
)


(
    w = Window.new.front;
    a = Button(w, Rect(10, 10, 200, 20)).states_([["Hi There!"]]);
    a.beginDragAction = { a.dragLabel ="I'm dragging: \""++ a.states[0][0]++"\""; a.states[0][0] };
    DragSink(w,Rect(10,40,200,20)).align_(\center).string="Cmd-drag from Button to here";
)

(
var value = 0.5;
w = Window.new.front;

// (1) create a UserView
v = UserView(w,Rect(50,50,200,20));

// (2) define a drawing function using Pen
v.drawFunc = {
    // Draw the fill
    Pen.fillColor = Color.grey;
    Pen.addRect(Rect(0,0, v.bounds.width*value,v.bounds.height));
    Pen.fill;
    // Draw the triangle
    Pen.fillColor = Color.red;
    Pen.moveTo(((v.bounds.width*value)-5) @ v.bounds.height);
    Pen.lineTo(((v.bounds.width*value)+5) @ v.bounds.height);
    Pen.lineTo(((v.bounds.width*value)) @ (v.bounds.height/2));
    Pen.lineTo(((v.bounds.width*value)-5) @ v.bounds.height);
    Pen.fill;
    // Draw the frame
    Pen.strokeColor = Color.black;
    Pen.addRect(Rect(0,0, v.bounds.width,v.bounds.height));
    Pen.stroke;
};

// (3) set the default action
v.action = {value.postln; v.refresh};

// (4) define mouse actions
v.mouseDownAction = { arg view, x = 0.5,y, m;
    //m.postln;
    ([256, 0].includes(m)).if{ // restrict to no modifier
    value = (x).linlin(0,v.bounds.width,0,1); v.doAction};
};

v.mouseMoveAction = v.mouseDownAction;

// (5) (optional) define key actions
v.keyDownAction = { arg view, char, modifiers, unicode,keycode;
    if (unicode == 16rF700, { value = (value+0.1).clip(0,1) });
    if (unicode == 16rF703, { value = (value+0.1).clip(0,1) });
    if (unicode == 16rF701, { value = (value-0.1).clip(0,1) });
    if (unicode == 16rF702, { value = (value-0.1).clip(0,1) });
    v.doAction;
};

// (6) (optional) define drag and drop behavior
v.beginDragAction = {value}; // what to drag
v.canReceiveDragHandler = {View.currentDrag.isNumber}; // what to receive
v.receiveDragHandler = {value = View.currentDrag; v.doAction }; // what to do on receiving


// just for testing drag and drop
Slider(w,Rect(50,100,200,20));

StaticText(w,Rect(50,150,350,50)).string_("To Test Drag and Drop,\nHold down Cmd (Ctl) Key");
)

(
w=Window.new.front;
Routine{
    20.do{
    w.bounds=Rect(200.rand, 200+200.rand, 300,300);
    0.1.wait;
    };
    w.close;
}.play(AppClock)
)

