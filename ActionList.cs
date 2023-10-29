using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using static Action;

// Global variable for our Delegate function to be called in Keyboard Input
public delegate void FuncCall(KeyCode key);
public delegate void VoidFunc();

public class ActionList : MonoBehaviour
{
    public List<Action> Actions;

    public float DTScale;
    private bool BlockingAll = false;

    void Start()
    {
        // initialize our Actions list
        Actions = new List<Action>();
        DTScale = 1.0f;
    }

    void Update()
    {
        // delta time variable
        float dt = Time.deltaTime * DTScale;

        // group of actions
        int BlockingGroupSet = 0;

        if (BlockingAll == true)
            return;

        // iterate through each action to update them
        for (int i = 0; i < Actions.Count; i++)
        {
            // Are we in the same blocking group?
            if (Actions[i].IsInGroupSet(BlockingGroupSet))
                continue;

            // Are we in a delay state?
            if (Actions[i].IncrementTime(dt) == false)
                continue;

            // Have we finished our action?
            if (Actions[i].Update(dt) == false)
            {
                // remove the action and continue
                Actions.RemoveAt(i);
                i--;
                continue;
            }

            // Are we blocking future actions?
            if (Actions[i].Blocking == true)
                BlockingGroupSet = BlockingGroupSet | Actions[i].Groups;
        }
    }

    // base function to move an object
    public Action MoveTo(GameObject moveObject, Vector3 start, Vector3 end, float duration, float delay, int group = 0, EaseType ease = EaseType.Linear, bool dynamic = false)
    {
        Move newAction = new Move(moveObject, start, end, duration, delay, group, ease, dynamic);
        Actions.Add(newAction);
        return newAction;
    }

    // simplified move function
    public Action MoveTo(GameObject moveObject, Vector3 end, float duration = 0.0f, float delay = 0.0f, int group = 0, EaseType ease = EaseType.Linear, bool dynamic = false)
    {
        Move newAction = new Move(moveObject, moveObject.transform.localPosition, end, duration, delay, group, ease, dynamic);
        Actions.Add(newAction);
        return newAction; 
    }

    // move function but with groups specified
    public Action MoveTo(GameObject moveObject, Vector3 end, float duration = 0.0f, int group = 0, EaseType ease = EaseType.Linear, bool dynamic = false)
    {
        Move newAction = new Move(moveObject, moveObject.transform.localPosition, end, duration, 0.0f, group, ease, dynamic);
        Actions.Add(newAction);
        return newAction;
    }

    public Action RotateTo(GameObject rotateObject, float start, float end, float duration = 0.0f, float delay = 0.0f, EaseType ease = EaseType.Linear, int group = 0)
    {
        Rotate newAction = new Rotate(rotateObject, new Vector3(0.0f, 0.0f, start), new Vector3(0.0f, 0.0f, end), duration, delay, group, ease);
        Actions.Add(newAction);
        return newAction;
    }

    public Action RotateTo(GameObject rotateObject, float end, float duration = 0.0f, float delay = 0.0f, EaseType ease = EaseType.Linear, int group = 0)
    {
        return RotateTo(rotateObject, rotateObject.transform.localEulerAngles.z, end, duration, delay, ease, group);
    }

    public Action RotateTo(GameObject rotateObject, float end, float duration = 0.0f, EaseType ease = EaseType.Linear, int group = 0)
    {
        return RotateTo(rotateObject, rotateObject.transform.localEulerAngles.z, end, duration, 0.0f, ease, group);
    }

    // Block all actions until completed
    public void Set_BlockAllActions(bool set)
    {
        BlockingAll = set;
    }

    // get whether we're blocking or not
    public bool Get_BlockAllActions()
    {
        return BlockingAll;
    }

    // reverse each action
    public void ReverseAll()
    {
        foreach (Action action in Actions)
        {
            action.Reverse();
        }
    }

    // scale the input object
    public Action ScaleObj(GameObject scaleObject, Vector3 startScale, Vector3 endScale, float duration = 0.0f, int group = 0, float delay = 0.0f, bool blocking = true, EaseType ease = EaseType.Linear)
    {
        Scale newAction = new Scale(scaleObject, startScale, endScale, duration, delay, group);
        newAction.Blocking = blocking;
        Actions.Add(newAction);
        return newAction;
    }

    public Action CardsFaceDown(GameObject card, float duration = 0.0f, int group = 0, float delay = 0.0f, bool blocking = true)
    {
        // the scales we want to go to
        Vector3 startScale = card.transform.localScale;
        Vector3 endScale = new Vector3(0.0f, startScale.y * 1.2f, startScale.z);

        // don't flip the cards if they're already face down
        if (card.transform.Find("Canvas/Text").gameObject.GetComponent<TMPro.TextMeshProUGUI>().alpha == 0.0f)
        {
            return null;
        }

        // create a new action to scale the card to a flat 1D plane, make it block next actions
        Scale newAction = new Scale(card, startScale, endScale, duration, delay * 0.5f, group);
        newAction.Blocking = blocking;
        Actions.Add(newAction);

        // create a new "display face" action to toggle the card's face
        DisplayFace newActionAlt = new DisplayFace(card, false, 0.0f, delay * 0.5f, group, Color.blue);
        newActionAlt.Blocking = false;
        Actions.Add(newActionAlt);

        // add another action to revert the card to its original state
        newAction = new Scale(card, endScale, startScale, duration, delay * 0.5f, group);
        newAction.Blocking = blocking;
        Actions.Add(newAction);
        return newAction;
    }

    public Action FlipCard(GameObject flipObject, float duration = 0.0f, int group = 0, float delay = 0.0f, bool blocking = true)
    {
        // the scales we want to go to
        Vector3 startScale = flipObject.transform.localScale;
        Vector3 endScale = new Vector3(0.0f, startScale.y * 1.2f, startScale.z);

        // create a new action to scale the card to a flat 1D plane, make it block next actions
        Scale newAction = new Scale(flipObject, startScale, endScale, duration, delay * 0.5f, group);
        newAction.Blocking = blocking;
        Actions.Add(newAction);

        bool showFace = flipObject.transform.Find("CardFace").gameObject.GetComponent<SpriteRenderer>().color == Color.white ? false : true;
        // create a new "display face" action to toggle the card's face
        DisplayFace newActionAlt = new DisplayFace(flipObject, showFace, 0.0f, delay * 0.5f, group, Color.blue);
        newActionAlt.Blocking = false;
        Actions.Add(newActionAlt);
        
        // add another action to revert the card to its original state
        newAction = new Scale(flipObject, endScale, startScale, duration, delay * 0.5f, group);
        newAction.Blocking = blocking;
        Actions.Add(newAction);

        return newAction;
    }

    public Action FlipHand(List<GameObject> hand, float duration = 0.0f, int group = 0, float delay = 0.0f, bool blocking = false)
    {
        if (hand == null || hand.Count == 0)
            return null;


        for (int i = 0; i < hand.Count - 1; i++)
        {
            FlipCard(hand[i], duration, group, delay, false);
        }

        return FlipCard(hand[hand.Count - 1], duration, group, delay, blocking);
    }

    public void ChangeCardColor(GameObject card, Color newColor, float duration = 0.0f, int group = 0, float delay = 0.0f, bool blocking = true)
    {
        if (card == null)
            return;

        Action newAction = new ChangeCardFaceColor(card, duration, delay, group, newColor);
        newAction.Blocking = blocking;
        Actions.Add(newAction);

    }

    public InputKeyStroke AddKeyStroke(KeyCode key, FuncCall method, float delay, int group = 0, bool blocking = false)
    {
        InputKeyStroke newAction = new InputKeyStroke(key, method, delay);
        Actions.Add(newAction);
        return newAction;
    }

    public DoVoidFunc AddVoidFunc(VoidFunc method, float delay = 0.0f, int group = 0)
    {
        DoVoidFunc newAction = new DoVoidFunc(method, delay, group);
        Actions.Add(newAction);
        return newAction;
    }

    public Fade FadeTo(GameObject fadeObject, float newAlpha = 1.0f, float duration = 0.0f, int group = 0, float delay = 0.0f, bool blocking = false)
    {
        Fade newAction = new Fade(fadeObject, newAlpha, duration, delay, group);
        newAction.Blocking = blocking;
        Actions.Add(newAction);
        return newAction;
    }

}

public class Action
{
    public float Delay;                         // Time before starting action
    public float TimePassed;                    // Time since starting action
    public float Duration;                      // How long we perform this action for
    public float Percent;                       // How far we are to completing this action
    public bool Blocking;                       // Are we blocking other actions?
    public int Groups;                          // What groups this object is in
    public EaseType Easing;                     // What type of easing will we use?
    public GameObject ActionObject = null;      // ID of the action

    public virtual bool Update(float dt)
    {
        // whether we've completed this action yet
        return Percent < 1.0f;
    }

    public enum EaseType
    {
        Linear, 
        EaseIn,
        EaseOut,
        FastIn,
        FastOut,
        InAndOut,
        Bounce
    };

    public float Ease(float percent, EaseType type)
    {
        if (percent <= 0.0f)
            return 0.0f;
        if (percent >= 1.0f)
            return 1.0f;

        switch (type)
        {
            case EaseType.Linear:   return percent;
            case EaseType.EaseIn:   return Mathf.Sqrt(percent);
            case EaseType.EaseOut:  return Mathf.Pow(percent, 2.0f);
            case EaseType.FastIn:   return Mathf.Sqrt(Mathf.Sqrt(percent));
            case EaseType.FastOut:  return Mathf.Pow(percent, 4.0f);
            case EaseType.InAndOut:
                if (percent < 0.5f)
                    return Mathf.Pow(percent * 2.0f, 2.0f) * 0.5f;
                else
                    return Mathf.Sqrt((percent - 0.5f) * 2.0f) * 0.5f + 0.5f;
            case EaseType.Bounce:
                float n = 7.5625f;
                float d = 2.75f;
                if (percent < 1.0f / d)
                    return n * percent * percent;
                else if (percent < 2.0f / d)
                    return n * (percent -= 1.5f / d) * percent + 0.75f;
                else if (percent < 2.5f / d)
                    return n * (percent -= 2.5f / d) * percent + 0.9375f;
                else
                    return n * (percent -= 2.625f / d) * percent + 0.984375f;
        }

        return percent;
    }

    public bool IncrementTime(float dt)
    {
        if (Delay > 0.0f)
        {
            // decrement delay
            Delay -= dt;

            // quit out if we still have a delay
            if (Delay > 0.0f)
            {
                return false;
            }

            // offset the time passed by whatever delay we have leftover
            TimePassed -= Delay;
            // reset the delay
            Delay = 0.0f;
        }
        else
        {
            // pass time
            TimePassed += dt;
        }

        if (TimePassed >= Duration)
        {
            // reset the time passed and percent
            TimePassed = Duration;
            Percent = 1.0f;
        }
        else
        {
            // calculate the percentage done
            Percent = TimePassed / Duration;
        }

        // change the percent based off the easing function applied
        Percent = Ease(Percent, Easing);

        // successful run
        return true;
    }

    // Add this action to the specified group number
    public void AddToGroup(int groupNum)
    {
        // if group is invalid, return
        if (groupNum < 1 || groupNum > 30)
            return;

        // Add the action to the group
        Groups = Groups | (1 << (groupNum - 1));
    }

    // Is this action in a specified group
    public bool IsInGroup(int groupNum)
    {
        // return if invalid group
        if (groupNum < 1 || groupNum > 30)
            return false;

        // find out if the object is in this group
        return (Groups & (1 << (groupNum - 1))) != 0;
    }

    // Determine if this object is in the group set for blocking
    public bool IsInGroupSet(int groupSet)
    {
        // if the group set is invalid, return out
        if (groupSet < 0)
            return false;

        // determine if the group set is in the master group set
        return (Groups & groupSet) != 0;
    }

    // Inverse the time passed
    public void ReverseTime()
    {
        // if there's a delay, do nothing
        if (Delay > 0)
            return;

        // flip the time passed
        TimePassed = Duration - TimePassed;

        // recalculate the percent
        Percent = TimePassed / Duration;

        // reapply the easing logic
        Percent = Ease(Percent, Easing);
    }

    // master reverse function
    public virtual void Reverse()
    {
        ReverseTime();
    }
}

public class Move : Action
{
    Vector3 Start;      // start position
    public Vector3 End;        // end position
    bool Dynamic;       // get start position on action start?

    // initialize every variable
    public Move(GameObject objectToMove, Vector3 start, Vector3 end, float duration, float delay, int group, EaseType ease = EaseType.Linear, bool dynamic = false)
    {
        ActionObject = objectToMove;
        Start = start;
        End = end;
        Duration = duration;
        Delay = delay;
        Easing = ease;
        Dynamic = dynamic;
        AddToGroup(group);
    }

    public override bool Update(float dt)
    {
        // null checker
        if (ActionObject == null)
            return false;

        // if we're a dynamic move
        if (Dynamic == true)
        {
            // update our start position for the first update frame
            Start = ActionObject.transform.localPosition;
            Dynamic = false;
        }    

        // move the object in the direction of the goal based off the percent we're done
        ActionObject.transform.localPosition = Start + (End - Start) * Percent;

        // return whether or not we've completed our task
        return Percent < 1.0f;
    }

    // Reverse the action
    public override void Reverse()
    {
        // if there's a delay, do nothing
        if (Delay > 0)
            return;

        // Switch the start and end positions
        Vector3 temp = Start;
        Start = End;
        End = temp;

        // reverse the time passed
        ReverseTime();
    }
}

public class Rotate : Action
{
    public Vector3 Start;
    public Vector3 End;

    public Rotate(GameObject objectToRotate, Vector3 start, Vector3 end, float duration = 0.0f, float delay = 0.0f, int group = 0, EaseType ease = EaseType.Linear)
    {
        ActionObject = objectToRotate;
        if (start.z - end.z > 180.0f)
                start.z -= 360.0f;
        if (start.z - end.z < -180.0f)
            end.z -= 360.0f;
        Start = start;
        End = end;
        Duration = duration;
        Delay = delay;
        Easing = ease;
        AddToGroup(group);
    }

    public override bool Update(float dt)
    {
        // can't rotate nothing
        if (ActionObject == null)
            return false;

        if (Start.z - End.z > 180.0f)
            Start.z -= 360.0f;
        if (Start.z - End.z < -180.0f)
            End.z -= 360.0f;

        // rotate the object 
        ActionObject.transform.localEulerAngles = Start + (End - Start) * Percent;

        // are we done yet?
        return Percent < 1.0f;
    }

    public override void Reverse()
    {
        // don't reverse if we haven't started yet
        if (Delay > 0)
            return;

        // Swap the start and end rotations
        Vector3 temp = Start;
        Start = End;
        End = temp;

        // flip our time variables
        ReverseTime();
    }
}

public class Scale : Action
{
    public Vector3 Start;
    public Vector3 End;

    // Rotate Action Constructor
    public Scale(GameObject objectToRotate, Vector3 startScale, Vector3 endScale, float duration, float delay, int group, EaseType ease = EaseType.Linear)
    {
        ActionObject = objectToRotate;
        Start = startScale;
        End = endScale;
        Duration = duration;
        Delay = delay;
        Easing = ease;
        AddToGroup(group);
    }

    public override bool Update(float dt)
    {
        // do nothing if we don't have an object
        if (ActionObject == null)
            return false;

        // apply a rotation of the object
        ActionObject.transform.localScale = (Start + (End - Start) * Percent);

        // do we continue?
        return Percent < 1.0f;
    }

    public override void Reverse()
    {
        // if there's a delay, do nothing
        if (Delay > 0)
            return;

        // Switch the start and end positions
        Vector3 temp = Start;
        Start = End;
        End = temp;

        Percent = 1 - Percent;

        // reverse the time passed
        ReverseTime();
    }
}

public class Fade : Action
{
    public float Start;
    public float End;

    // Rotate Action Constructor
    public Fade(GameObject objectToFade, float endScale, float duration, float delay, int group, EaseType ease = EaseType.Linear)
    {
        ActionObject = objectToFade;
        Start = objectToFade.transform.GetComponent<Image>().color.a;
        End = endScale;
        Duration = duration;
        Delay = delay;
        Easing = ease;
        AddToGroup(group);
    }

    public override bool Update(float dt)
    {
        // do nothing if we don't have an object
        if (ActionObject == null)
            return false;

        Color currColor = ActionObject.transform.GetComponent<Image>().color;
        Color currColor_Text = ActionObject.transform.Find("Text").GetComponent<TMPro.TextMeshProUGUI>().color;

        // apply a rotation of the object
        ActionObject.transform.GetComponent<Image>().color = new Color(currColor.r, currColor.g, currColor.b, Start + (End - Start) * Percent);
        ActionObject.transform.Find("Text").GetComponent<TMPro.TextMeshProUGUI>().color = new Color(currColor_Text.r, currColor_Text.g, currColor_Text.b, Start + (End - Start) * Percent);


        // do we continue?
        return Percent < 1.0f;
    }

    public override void Reverse()
    {
        // if there's a delay, do nothing
        if (Delay > 0)
            return;

        // Switch the start and end positions
        float temp = Start;
        Start = End;
        End = temp;

        Percent = 1 - Percent;

        // reverse the time passed
        ReverseTime();
    }
}

public class DisplayFace : Action
{
    bool ShowFace;
    Color Color;

    // Rotate Action Constructor
    public DisplayFace(GameObject cardObject, bool showFace, float duration, float delay, int group, Color color)
    {
        ActionObject = cardObject;
        ShowFace = showFace;
        Duration = duration;
        Delay = delay;
        Color = color;
        AddToGroup(group);
    }

    public override bool Update(float dt)
    {
        // do nothing if we don't have an object
        if (ActionObject == null)
            return false;

        // toggle face display
        if (ShowFace == false)
        {
            // fade out the text based off the percent
            ActionObject.transform.Find("Canvas/Text").gameObject.GetComponent<TMPro.TextMeshProUGUI>().alpha = (255.0f - (Percent * 255.0f));
            // if we're at the end, change the color of the card face
            ActionObject.transform.Find("CardFace").gameObject.GetComponent<SpriteRenderer>().color = Color;
        }
        else if (ShowFace == true)
        {
            // fade in the text based off the percent
            ActionObject.transform.Find("Canvas/Text").gameObject.GetComponent<TMPro.TextMeshProUGUI>().alpha = (0.0f + (Percent * 255.0f));
            // at the end of the action, change the card face back to white
            ActionObject.transform.Find("CardFace").gameObject.GetComponent<SpriteRenderer>().color = Color.white;
        }
        

        // do we continue?
        return Percent < 1.0f;
    }

    public override void Reverse()
    {
        // if there's a delay, do nothing
        if (Delay > 0)
            return;

        // Switch the start and end positions
        ShowFace = !ShowFace;

        // reverse the time passed
        ReverseTime();
    }
}

public class ChangeCardFaceColor : Action
{
    Color Color;

    // Rotate Action Constructor
    public ChangeCardFaceColor(GameObject cardObject, float duration, float delay, int group, Color color)
    {
        ActionObject = cardObject;
        Duration = duration;
        Delay = delay;
        Color = color;
        AddToGroup(group);
    }

    public override bool Update(float dt)
    {
        if (ActionObject == null)
            return false;

        ActionObject.transform.Find("CardFace").gameObject.GetComponent<SpriteRenderer>().color = Color;

        return false;
    }

}

public class InputKeyStroke : Action
{
    KeyCode input;
    public FuncCall methodToCall;

    // initialize every variable
    public InputKeyStroke(KeyCode keyval, FuncCall method, float delay, int group = 0, bool blocking = false)
    {
        ActionObject = null;
        Duration = 0.0f;
        Delay = delay;
        Easing = EaseType.Linear;
        AddToGroup(group);
        input = keyval;
        methodToCall = method;
        Blocking = blocking;
    }

    public override bool Update(float dt)
    {
        // call our Key Input action
        methodToCall(input);

        // done with what we need to do
        return false;
        // return whether or not we've completed our task
        //return Percent < 1.0f;
    }

    // Reverse the action
    public override void Reverse()
    {
        // if there's a delay, do nothing
        if (Delay > 0)
            return;

        // reverse the time passed
        ReverseTime();
    }
}

public class DoVoidFunc : Action
{
    public VoidFunc methodToCall;

    // initialize every variable
    public DoVoidFunc(VoidFunc method, float delay = 0.0f, int group = 0)
    {
        ActionObject = null;
        Duration = 0.0f;
        Delay = delay;
        Easing = EaseType.Linear;
        AddToGroup(group);
        methodToCall = method;
    }

    public override bool Update(float dt)
    {
        // call our Key Input action
        methodToCall();

        // done with what we need to do
        return false;
        // return whether or not we've completed our task
        //return Percent < 1.0f;
    }

    // Reverse the action
    public override void Reverse()
    {
        // if there's a delay, do nothing
        if (Delay > 0)
            return;

        // reverse the time passed
        ReverseTime();
    }
}