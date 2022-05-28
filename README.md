## Behavior Scripting 101

#### Introduction
This Tutorial is a starting point to start scripting on the behavior engine in a fun and practical way. 
Together we will be implementing a simple alarm system that can run on the behavior engine. 
Along the way we will be introducing different concept and features that are needed to script your own use-cases.

#### Scope
The main goal of this tutorial is to:
* Guide you how to use the tooling provided by the Behavior SDK.
* Guide you through the api documentation.
* Help you get up and running with behavior scripting.

#### Use-Case: The Hue Alarm
The use-case for our behavior script is going to be a Hue Alarm. The alarm system can:
* Arm automatically when the users are all out of home.
* Disarm automatically when some user arrives home.
* Trigger an alarm (light siren) when movement, i.e. burglar, is detect in you home.
* Enabling cancellation of the alarm, using an app.

#### Setup the project
We will start with by creating a new project and add your own script.
1. Open up the Webstorm IDE and create a new project. 
2. Within this empty project open up a terminal and run the init_empty_git_project script.
    ```
    ./<PATH_TO_SDK>/hue-behaviord-sdk/tools/project_utils/init_empty_git_project.sh
    ```
    This will setup the structure of your project and cloning some tooling inside the **3rd_party** folder.
3. Add a new script to your project called HueAlarm.

    ```
    npm run add-script HueAlarm
    ```
    This generates a hue_alarm.js and hue_alarm.json file in the **src/scripts** folder.
    The hue_alarm.js file is the actual behavior script and will already contain the skeleton needed to run it.
    The hue_alarm.json file is the script definition, containing the script_id used for run the script, the name, the description and the configuration/trigger schema.
    
    When opening up the hue_alarm.js file you will the following.
    ```javascript
    function HueAlarm(config) {
        // ToDO: update definition file
        this.config = config;
        
        this.log.debug("I am ready!");
        // ToDO: your initialization code here.
        // ...
    }
    ```
   This is the constructor of you script. After running the script in you bridge (i.e creating an instance of the script), 
   this function is the first one to be called.
4. Generate the script API documentation    
    ```
    // todo: fill in.
    ```
   
#### Your home setup
In order for you to test your script you will need a home setup. 
1. Modify you home setup to your liking. We will be using a setup for a small apartment.
    ```
    // todo: fill in.
    ```
2. Setup and run your hse.
    ```
    // todo: fill in.
    ```
   Using a **web browser** you are able to see your hse by going to http://localhost.\
   In the **Run** tab you are able to see the logging information of the bridge.
3. Create a user and RLC your the devices in the 
    ```
    // todo: fill in.
    ```
4. Do Restfull commands to your hse bridge, make use of the HTTP request tooling the Webstorm provides.
    ```
    // todo: fill in.
    ```
    Off course if you want to make use of other tooling like Postman, that is also fine.  
5. To have your script and HSE visible on your IDE, install a web browser in Webstorm. This will drastically improve you scripting experience since everything what you need is on one screen.
    ```
    1. Go to File -> Setting -> Plugins. 
    2. Search for "Gideabrowser" and install it.
    3. Open up the browser and go to http://localhost
    ``` 

#### Running your first behavior script
After starting up the HSE the hue_alarm.js script is already available in the emulated bridge.
1. Do request for available scripts in you HSE
    ```
    1. Open <PROJECT>/http_client/resource/behavior_script.http file
    2. Go to the  "### get script by type" section
    3. Press the green "play" button to do the GET request. 
    ``` 
    This will do a request for the available scripts in the HSE bridge. In the response, we can find our 
    script definition.
    
    todo: update, schema should be empty
    ```json
    { 
        "data": [{
            "configuration_schema": {},
            "description": "hue system alarm",
            "id": "52531087-816b-4440-972a-4f287ccb83e9",
            "id_v1": "",
            "metadata": {
                "name": "hue_alarm",
                "type": "ScriptMetadata"
            },
            "state_schema": {},
            "trigger_schema": {},
            "type": "behavior_script",
            "version": "0.0.1"
        }],
        "errors": [],
        "type": "ClipMessageBehaviorScript"
    }
    ```
    Note that the name, description and id are from the script definition (json) file.
2. Run the HueAlarm in you HSE.
    ```
    1. Open <PROJECT>/http_client/hue_alarm.http file. 
    2. Go to the "### create wakeup" section
    3. Press the green "play" button to do the POST request. 
    ```
    This will do a HTTP POST to the emulated bridge installing a new [instance]() of the HueAlarm script and runs it.
    In the log tab, you will find both that the script has been installed and the *"I am ready!"* logs from 
    the constructor the HueAlarm script.
    
    Your first script is up and running! Now let's begin filling this script up with the use-cases.
#### Do light control using the clip module
In order to control your lights, the script needs to know *which* lights to control and *how* to control them. 
This section will introduce two new concepts. The first one is the [configuration](). Within this configuration
we can decide which light should participate in you HueAlarm. The second concept is the [clip module](https://www.google.com). 
Using the clip module, the script is able to do [clip v2](https://www.code.dtf.lighting.com/projects/HSYS/repos/clip-specification) 
requests within the HSE bridge.

1. Query the lights in your setup.
    ```
    1. open <PROJECT>/http_client/resource/light.http file. 
    2. go to the "###Get lights" section
    3. press the green "play" button to do the GET request
    ```
   The clip response of this request is shown in the run tab. Here we can find the available light and their capabilities. 
   Decide which lights should participate in the Hue Alarm and save the id's.
2. Configure the hue system alarm.
    ```
    1. open <PROJECT>/http_client/hue_alarm.http file. 
    2. go to the "###Create Hue Alarm" section
    3. update the configuration attribute of the request body like this:
    
        {
          "type": "behavior_instance",
          "script_id": "<hue_alarm_script_id>",
          "enabled": true,
          "configuration": {
            "lights": [
              {
                "rid": "<light_service_id1>",
                "rtype": "light"
              },
              {
                "rid": "<light_service_id2>",
                "rtype": "light"
              }
            ]
          }
        }
   
3. In the constructor of HueAlarm script add the following logging. This stringifies the configuration
object that  is passed in the script constructor and prints it.
    ```
    this.log.debug("configuration: " + JSON.stringify(config));
    ```
    The configuration will be shown in the logs when the script will be run.
    
4. Control your lights using the [clip module](...). We turn **off** all lights that participate in the HueAlarm 
   by adding the following code snippet to the constructor:
    ```javascript
    const self = this;
    var setLightActions = function(light, action) {
        self.clip.put('/' + light.rtype + '/' + light.rid,
            action
            , function (response) {
                self.log.debug("response:" + JSON.stringify(response));
            });
    }
   
    this.config.forEach(function(light) {
        var offAction = { on: { on: false } };
        setLightActions(light, offAction);
    });
    ```
    Note: since the callback is asynchronous, we need to capture "this" in a different variable "self". 
5. The modified script needs to be reloaded in the HSE
    ```
    todo
    ```
6. Run you HueAlarm script/
#### Start the alarm siren using timers
The when alarm triggers we want a light siren effect scaring off the burglar. The lights participating
in the HueAlarm should continuously blink from red to blue.
1. Create the red and blue light action. Specification an action can be found in the [clip specifications](https://www.code.dtf.lighting.com/projects/HSYS/repos/clip-specification/browse/raml).
    ``` 
    const blueAction = {
        color: {
            xy: {
                x: 0.15,
                y: 0.11
            }
        },
        dimming: {
            brightness: 100
        },
        on: {
            on: true
        }
    };
   
    const redAction = {
        color: {
            xy: {
                x: 0.67,
                y: 0.31
            }
        },
        dimming: {
            brightness: 100
        },
        on: {
            on: true
        }
   };
    ```
2. Add a [recurring timer](...) to the script that will triggers every second.
    ```javascript
    this.timer.setRecurringAlarm(function() { this.log.debug("tick") }, "* * * * * *");
    ```
    Note that second argument "* * * * * *" is string representing a [cron expression](https://www.freeformatter.com/cron-expression-generator-quartz.html) schedule for every second.
    After running the script you will see "tick" be printed out in the log every second.
3. Now bring it all together by replacing the previous code snippet with the following one.
    ```javascript
    self.turnOnAlarm = function () {
        self.state = self.blueAction;
        self.timerId = self.timer.setRecurringAlarm(function() {
            if (self.state === blueAction) {
                self.state = redAction;
            } else {
                self.state = blueAction;
            }
            self.config.lights.forEach(function(light) {
                setLightActions(light, self.state);
            });
        }, "* * * * * *");
    }
   
    self.turnOnAlarm();
    ```
4. Reload and run the script. The paricipating lights blinks between blue and red every second.
5. To cancel this recurring timer, we have saved the timer id into a variable. To support functionality for canceling the
timer we add the following code snippet to the script.
    ```javascript
    self.turnOffAlarm = function () {
        if(self.timerId !== undefined){
            self.timer.cancelTimer(self.timerId);
            self.timerId = undefined;
        }
    }
    ```
   This simply cancels the timer that got created with the timer id.
#### Trigger Alarm when motion detected
Now, the light siren starts when the HueAlarm script starts to run. However, what we want is that the siren only 
starts when motion is detected in you home.

1. Add a motion sensor to your configuration. 
    ```
    1. open <PROJECT>/http_client/resource/motion.http file. 
    2. go to the "###Get motion" section
    3. press the green "play" button to do the GET request
    4. save the <motion_sensor_id> that you want to participate in the HueAlarm.
    5. open <PROJECT>/http_client/hue_alarm.http file. 
    6. add the motion_sensor to the json configuration in the request body:
    
        {
          "type": "behavior_instance",
          "script_id": "<hue_alarm_script_id>",
          "enabled": true,
          "configuration": {
            "motion_sensor": {
                "rid": "<motion_sensor_id>",
                "rtype": "motion"
            },
            "lights": [
                {
                    "rid": "<light_service_id1>",
                    "rtype": "light"
                },
                {
                    "rid": "<light_service_id2>",
                    "rtype": "light"
                }
            ]
          }
        }
    ```
2. Subscribe to changes in the motion sensor and turn on the alarm when motion is true.
    ```javascript
    var motionHandler = function (serviceId, motionEvent) {
        self.log.debug("motion: " + JSON.stringify(motionEvent));
        if (motionEvent[0].motion.motion === true) {
            self.turnOnAlarm();
        }
    }
    self.subscribe_update_service(self.config.motion_sensor.rid, motionHandler);
    ```
#### Arm and disarm your alarm using geofencing
The alarm should only be armed when the user leave his/her home. We can make use of the 
[geofencing](https://en.wikipedia.org/wiki/Geo-fence) functionality where
the users mobile phone (app) represents and geofence client and update the bridge when it enters the geofence. 

1. Subscribe to the [geofence](todo)
    ```
    self.geofence.subscribe(function(geofenceEvent) {
        self.log.debug("geofenceEvent:" + JSON.stringify(geofenceEvent));
    });
    ```
   The geofence event specification can be found [here](https://www.code.dtf.lighting.com/projects/HSYS/repos/clip-specification/browse/raml/GeofenceLib.raml#27)
2. Simulate user entering or leaving the geofence.
    ```
    1. open <PROJECT>/http_client/geofence.http file. 
    2. go to "### create geofence_client" section.
    3  modify the "is_at_home" attribute in the PUT body to either true or false.
    4. press the green play button.
    ```
   The logs shows the geofenceEvent object, when geofence clients is_at_home attribute changes.
3. Add the logic to [subscribing]() and [unsubscribing]() to the motion sensor updates. Script should subscribe to updates when 
"any_at_home" is false and unsubscribe when "any_at_home" is true.
    ```
    self.geofence.subscribe(function(geofenceEvent) {
        self.log.debug("geofenceEvent:" + JSON.stringify(geofenceEvent));
        if (geofenceEvent.any_at_home === false) {
            self.subscribe_update_service(self.config.motion_sensor.rid, motionHandler);
        } else {
            self.unsubscribe_update_service(self.config.motion_sensor.rid);
        }
    });
    ```
#### Cancel the alarm using script triggers.
When alarm is on (lights are blinking), user should be able to turn off the alarm using the HueApp. Bridge enables this 
by offering [script triggers](todo). Using script triggers we are able to control a specific running instance of a HueAlarm.
 
1. Uncomment the [prototype.trigger]() section of the HueAlarm script and add a log.
    ```javascript
    HueAlarm.prototype.trigger = function (trigger) {
        self = this;
        self.log.debug("script trigger:" + JSON.stringify(trigger));
    }
   ``` 
2. This part of the code can be invoked using the script triggers
   ```
    1. run the HueAlarm script and save the instance id.
    2. open <PROJECT>/http_client/hue_alarm.http file.
    3. go to "### script trigger" section.
    4. update {{instance id}} to the instance id of the running 
    5. modify the request body to:
   
       {
         "trigger": {
           "stop":{}
         }
       }

    6. press the green play button of this section.
   ```
   The logs shows the trigger object, when the HueAlarm script instance receives the scrip trigger.
3. Turn off the alarm when trigger is received.
    ```javascript
    HueAlarm.prototype.trigger = function (trigger) {
        self = this;
        self.log.debug("script trigger:" + JSON.stringify(trigger));
        if (trigger.stop) {
            self.turnOffAlarm();
        }
    }
    ``` 
#### Where to go from here.
Congratulations! You have just created your first behavior script. 
Off course, it does not end here. We still have many topics that we still need to cover before this Hue Alarm System is production ready.
* Have you noticed that the lights are not blinking in sync when the alarm is on? We can solve this *popcorn effect* using private resources.
* What about the configuration that the AlarmSystem Script gets in the constructor. The script is not checking for validity i.e. does the configuration contain any lights or the motion sensors? 
We can make use of proper schema validation to check the configuration.
* What about testing? We have the Jest Testing framework ready in the project to be used.
* And what about memory consumption of your script? Are you using too much memory?

This and more will be covered by **Behavior Scripting 102: Making Your Script Production Ready**.

 



