using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using UnityEngine.SceneManagement;

public class Playback : MonoBehaviour
{
    public int seed;
    public ActionList playbackActions;

    string w_filepath;
    string r_filepath;
    public float timeSinceStart = 0.0f;
    public bool record = true;

    private bool once = false;

    StreamWriter writer;
    StreamReader reader;

    private void Awake()
    {
        // count the number of playback objects in the scene
        int numPlaybacks = FindObjectsOfType<Playback>().Length;

        // if we already have a playback instance, delete ourselves. Otherwise, save us for when we reload the scene to playback
        if (numPlaybacks != 1)
        {
            record = false;
            GameObject.Find("UI_Manager").GetComponent<UIManager>().replayMode = true;
            GameObject.Find("UI_Manager").GetComponent<UIManager>().ClearActionList();
            GameObject.Find("UI_Manager").GetComponent<PauseMenuManager>().replayMode = true;
            Destroy(this.gameObject);
        }
        else
        {
            record = true;
            DontDestroyOnLoad(gameObject);
        }
    }

    // Start is called before the first frame update
    void Start()
    {
        // Write filepath (don't override the previous data)
        w_filepath = "./current_game.txt";
        // Read filepath (read the previous game)
        r_filepath = "./previous_game.txt";

        // if we already played a game
        if (File.Exists(w_filepath))
            // transfer that data over to the "previous game" file
            File.Copy(w_filepath, r_filepath, true);

        // start recording input
        record = true;

        // open up a new writer file
        writer = new StreamWriter(w_filepath, false);

        // create a new action list for keyboard input
        playbackActions = Instantiate(Resources.Load<ActionList>("ActionList"), new Vector3(0, 0, 0), Quaternion.identity);
        DontDestroyOnLoad(playbackActions);
    }

    // Update is called once per frame
    void Update()
    {
        // if we're in a playback and hit escape, quit the game
        if (Input.GetKeyDown(KeyCode.Escape) && record == false)
        {
            Application.Quit();
        }

        // increment the time since application launch
        timeSinceStart += Time.deltaTime;

        // if we switch to playback mode
        if (Input.GetKeyDown(KeyCode.R) && once == false)
        {
            // prepare for replay sequence
            PlayBackActions();

            // clear all action lists to prevent any errors
            GameObject.Find("UI_Manager").GetComponent<UIManager>().ClearActionList();
            GameObject.Find("UI_Manager").GetComponent<PauseMenuManager>().ClearActionList();

            // reload the current scene
            SceneManager.LoadScene(SceneManager.GetActiveScene().name);
        }

        // if we're recording input this session
        if (record == true)
        {
            RecordKeyStrokes();
        }
    }

    void RecordKeyStrokes()
    {
        // loop through each key on the keyboard (and mouse)
        foreach (KeyCode key in System.Enum.GetValues(typeof(KeyCode)))
        {
            // if that key was pressed
            if (Input.GetKeyDown(key))
            {
                // write to file what key was pressed and the time since application launch
                writer.WriteLine(key.ToString() + "," + timeSinceStart.ToString());
            }
        }
    }

    void PlayBackActions()
    {
        // no longer writing input
        writer.Close();

        // disable manual player input when we replay a previous game
        GameObject.Find("UI_Manager").GetComponent<UIManager>().SetReplayMode();
        GameObject.Find("UI_Manager").GetComponent<PauseMenuManager>().SetReplayMode();

        // reset our time since start
        timeSinceStart = 0.0f;

        // variables for reading input
        string action;
        KeyCode key;
        float time;

        // open our input file
        reader = new StreamReader(r_filepath);

        if ((action = reader.ReadLine()) != null)
        {
            seed = int.Parse(action);
            // seed the random number generator from the seed
            //Random.InitState(int.Parse(action));
        }
        
        // read each line
        while ((action = reader.ReadLine()) != null)
        {
            // split the line into two strings separated by commas
            string[] currLine = action.Split(',');

            // cast the first string into a keycode
            key = (KeyCode)System.Enum.Parse(typeof(KeyCode), currLine[0]);
            // cast the second string into a float
            time = float.Parse(currLine[1]);

            // add a KeyStroke event to the action list
            playbackActions.AddKeyStroke(key, ResolveKeyStrokeOnAll, time);
        }

        // close our file
        reader.Close();

        // we're not recording and don't do this again
        record = false;
        once = true;
    }

    private void OnApplicationQuit()
    {
        // if we have a writer stream open, close it
        if (writer != null)
            writer.Close();

        // if we have a reader stream open, close it
        if (reader != null)
            reader.Close();
    }

    public void ResolveKeyStrokeOnAll(KeyCode key)
    {
        GameObject.Find("UI_Manager").GetComponent<UIManager>().KeyStrokeResolver(key);
        GameObject.Find("UI_Manager").GetComponent<PauseMenuManager>().KeyStrokeResolver(key);
    }

    public void SeedRandom()
    {
        if (record == true)
        {
            seed = Random.Range(1, 1001);
            Random.InitState(seed);
            // save out the seed
            writer.WriteLine(seed);
            print("Current Seed: " + seed.ToString());
        }
        else
        {
            Random.InitState(seed);
            print("Previous Seed: " + seed.ToString());
        }
    }
}
