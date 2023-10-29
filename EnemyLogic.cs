using KBT;
using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.AI;

public abstract class EnemyLogic : MonoBehaviour
{
    public bool debugOutput = false;            // Debug print states as they happen
    public float maxHealth;                     // Enemy's max  health
    [HideInInspector]
    public float health = 1.0f;                 // Enemy's current health
    public float damageDealt;                   // How much damage we deal
    public float damageDelay = 0.5f;            // How long do we wait after taking damage before taking damage again
    public Tree BT;                             // a reference to the Enemy's Behavior Tree GameObject
    public GameObject ExplosionVFX;             // reference to an explosion VFX prefab
    public GameObject DustVFX;                  // reference to an explosion VFX prefab
    public bool SpawnVFX = false;               // do we spawn VFX
    public bool FallFromSky = false;            // do we fall from the sky
    public float activationDelayTime = 3.0f;    // how long do we wait once we hit the ground before activating
    public float fallSpeed = 1.0f;              // how fast we fall
    public Vector3 explosionScale;
    [HideInInspector]
    public SpawningSystem masterSpawner = null; // the spawn logic that created this enemy

    public GameObject fallParticles = null;     // reference to the falling particle system

    private bool doOnce = false;                // used for spawning logic
    private bool spawnLogic = false;            // perform the spawn logic
    private bool increaseFallSpeed = false;     // true if we're falling from the sky
    private float spawnTimer = 0.0f;            // current timer for "activationDelayTime" variable
    private float currDamageDelay;              // How long until we take damage again


    // Start is called before the first frame update
    void Start()
    {
        OnSpawn();
    }

    // Update is called once per frame
    void Update()
    {
        OnUpdate(Time.deltaTime);
    }

    // events called when the enemy should die
    private void DeathSequence()
    {
        OnDeath();
        if (masterSpawner != null)
            masterSpawner.OnEnemyDeath(gameObject);
        Destroy(gameObject);
    }

    // Base death functionality
    virtual public void OnDeath()
    {
        // Print out that we're destroying the game object
        if (debugOutput)
            Debug.Log("Destroying " + transform.parent.name);

        GameObject player = GameObject.FindGameObjectWithTag("Player");
        if (player != null)
        {
            PlayerStats stat = player.GetComponent<PlayerStats>();
            if (stat != null)
            {
                //stat.AddPoints(100);
            }
        }

        // spawn an explosion at our location and make it so that it despawns after it finishes
        GameObject explosion = GameObject.Instantiate(ExplosionVFX, transform.position, Quaternion.identity);
        explosion.transform.localScale = explosionScale;
        var main = explosion.GetComponent<ParticleSystem>().main;
        main.loop = false;
        main.stopAction = ParticleSystemStopAction.Destroy;

        foreach (var child in explosion.GetComponentsInChildren<ParticleSystem>())
        {
            var cMain = child.GetComponent<ParticleSystem>().main;
            cMain.loop = false;
        }
    }

    // Called when a collision occurs
    private void OnCollisionEnter(Collision collision)
    {
        CollisionEnterLogic(collision);
    }

    virtual public bool CollisionEnterLogic(Collision collision)
    {
        // grab the terrain layer mask 
        int terrainLayer = LayerMask.NameToLayer("Terrain");

        // collided with the terrain
        if (collision.gameObject.layer == terrainLayer)
        {
            // if this is the first time and we're falling from the sky
            if (doOnce == false && FallFromSky == true)
            {
                // do we spawn VFX?
                if (SpawnVFX == true)
                {
                    // spawn a dust explosion VFX and make it despawn once it completes
                    GameObject explosion = GameObject.Instantiate(DustVFX, transform.position, Quaternion.identity);
                    explosion.transform.localScale = new Vector3(7.0f, 7.0f, 7.0f);
                    var main = explosion.GetComponent<ParticleSystem>().main;
                    main.loop = false;
                    main.stopAction = ParticleSystemStopAction.Destroy;

                    foreach (var child in explosion.GetComponentsInChildren<ParticleSystem>())
                    {
                        var cMain = child.GetComponent<ParticleSystem>().main;
                        cMain.loop = false;
                    }

                    // destroy the fall particles
                    if (fallParticles != null)
                        Destroy(fallParticles);
                }

                // start a delay for our enemy to activate
                spawnTimer = activationDelayTime;
                // start doing the spawn logic
                spawnLogic = true;
                doOnce = true;

                // enable our NavMesh Agent
                gameObject.GetComponent<NavMeshAgent>().enabled = true;
            }
        }




        // DEBUG LOGIC
        // If we collide with a missile, just die for right now
        if (collision.gameObject.GetComponent<GuidedMissile>() != null)
        {
            health = 0;
            return false;
        }

        // safe for child to run their logic
        return true;
    }


    virtual public void OnSpawn()
    {
        // update our health to the max health
        health = maxHealth;

        // if we're falling, increase our gravity
        if (FallFromSky == true)
        {
            BT.enabled = false;
            increaseFallSpeed = true;
            if (fallParticles != null)
            {
                fallParticles.SetActive(true);
            }
        }
        // if we're not falling from the sky, enable our agent and BT
        else
        {
            if (fallParticles != null)
                DestroyImmediate(fallParticles);
            BT.enabled = true;
            GetComponent<NavMeshAgent>().enabled = true;
        }

        GameObject player = GameObject.FindGameObjectWithTag("Player");
        BT.blackboard.SetVar(player, "player");
    }

    abstract public void DoDamage(GameObject damagedEntity);

    virtual public void TakeDamage(float damageTaken, GameObject damageSource)
    {
        // don't take damage if we are immune
        if (currDamageDelay != 0)
        {
            if (debugOutput)
                Debug.Log("Damage immunity for " + currDamageDelay + " seconds");
            return;
        }  
        
        // deal damage
        health -= damageTaken;


        // is this enemy dead or not?
        if (health <= 0)
        {
            if (debugOutput)
                Debug.Log("Enemy " + transform.parent.name + " has died. Begin death behavior");
            //DeathSequence();
        }
        else
        {
            if (debugOutput && damageSource != null)
                Debug.Log("Took " + damageTaken + " damage from " + damageSource.name);
            currDamageDelay = damageDelay;
        }
    }

    virtual public void OnUpdate(float dt) 
    {
        // decrement damage immunity timer until it reaches 0
        if (currDamageDelay > 0)
        {
            currDamageDelay -= dt;
        }
        else if (currDamageDelay < 0)
            currDamageDelay = 0;

        // run a timer as soon as we hit the terrain
        if (spawnLogic == true)
        {
            // when the time is up, enable our BT and escape out
            if (spawnTimer <= 0.0f)
            {
                BT.enabled = true;
                spawnTimer = 0.0f;
                spawnLogic = false;
            }
            else
            {
                spawnTimer -= dt;
            }
        }

        // if we want to increase the fall speed, add a force while we fall
        if (increaseFallSpeed == true)
        {
            GetComponent<Rigidbody>().AddForce(new Vector3(0.0f, -fallSpeed, 0.0f));
        }

        if (health <= 0)
        {
            DeathSequence();
        }
    }

}
