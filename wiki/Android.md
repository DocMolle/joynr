# joynr Android Developer Guide

## Initial Note

This guide is a work in progress as the joynr Android api has not been finalized.

## Setting up a joyner deployment

The process of setting up the joynr deployment is very similar to the one [Java](./java.md#setting-up-a-joynr-deployment) 
proccess.

## Android Development

As this is an Android Application it should be developed using the Android Developer [guidelines](https://developer.android.com/guide/).
Also when developing your application architecture you should keep in mind the best practices and  
recommended architectures [guide](https://developer.android.com/jetpack/docs/guide).

In order to see an example implementation you can check [android-hello-world](../examples/android/android-hello-world/README.md)  
provider and consumer examples for a more concrete example of implementing an Android joynr  
application using MVVM pattern.

See the [Java Configuration Reference](JavaSettings.md) for a complete list of all available  
configuration properties available to use in joynr Java applications, which has very simillar  
classes to Android.

## Setting up Android project environment in Android Studio

To develop an Android project with Android Studio first you need to create a new project following  
the guidelines in ["Create an Android Project"](https://developer.android.com/training/basics/firstapp/creating-project)
.

### Setting up Graddle

In order to use joynr java code generator you need to add the generator gradle plugin and the java  
generator in the project build.gradle, as these need to be applied in the app.

```gradle
	dependencies {
		...
		classpath "io.joynr.tools.generator:joynr-generator-gradle-plugin:<version>"
		classpath "io.joynr.tools.generator:java-generator:<version>"
		...
	}

```

> Note: 
> The version tag needs to be replaced with the version you want to use, check [here](./ReleaseNotes.md) 
> the available versions

Then in the application build.gradle the joynr generator gradle plugin needs to be applied in order  
to generate the joynr code through the fidl file. This can be done by adding the following line in  
the beginning of your app/build.gradle

```gradle
	apply plugin: 'io.joynr.tools.generator.joynr-generator-gradle-plugin'
```

And now in the dependencies you can add the libjoynr-android-websocket-runtime dependency in order  
use it in your program. For this in app/build.gradle you should add the following lines:  

```gradle
dependencies {
	...
	implementation "io.joynr.android:libjoynr-android-websocket-runtime:<version>"
	...
}
```

> Note: 
> The version tag needs to be replaced with the version you want to use, check [here](./ReleaseNotes.md) 
> the available versions
### The Application class

In an Android project it is recommended to use an class that exetends from the Application class  
and use that class as a singleton to manage the joynr runtime, just like in the following example.

```java
public class JoynrApplication extends Application {

	public JoynrRuntime runtime;

	public JoynrRuntime getRuntime() {
		return runtime;
	}

	public void onCreate() {
		...
		runtime = joynrInjector.getInstance(JoynrRuntime.class);
		...
	}
}
```

Using this approach you can create the runtime on the program initialization and access it anywhere  
in your application.



### The Model Class

The model for a consumer and a provider will be different as each one has a different reaction to  
data. The model from the consumer part will query the provider for data, in this case using  
assynchronous methods, while the provider receives the query and does the necessary operations to  
respond to the query that the consumer made.


#### The Consumer Model

This example consumer Model was made to update a string with every query made to the provider.  

Using the runtime created in the begining of the program a proxy is created in the initialization  
process, allowing the consumer to query to the provider. Since the runtime is stored in the   
[Application class](#the-application-class) it needs to be passed to the model, we can do this  
through the [ViewModel](#the-modelview-class)

```java
private HelloWorldProxy proxy;

public void setApp(final HelloWorldConsumerApplication app) {
	this.app = app;
}

public void init() {
	...
	proxy = app.getRuntime().getProxyBuilder("domain", HelloWorldProxy.class)
			.setMessagingQos(new MessagingQos()).setDiscoveryQos(discoveryQos).build();
	...
}
```

The proxy requests have a similiar api to Java [synchronous](#synchronous-remote-procedure-calls) 
and [assynchronous](#asynchronous-remote-procedure-calls) procedures.  

This data is linked with to the view model, using live data. This means that when the value of the  
string is updated an event is triggered that notifies the ViewModel.

```java
public MutableLiveData<String> text = new MutableLiveData<>();

private void updateString() {
	...
	text.post(futureString.get());
	...
}
```

#### The Provider Model

The provider Model in this example is just a provider manager and a bridge bettween the provider  
and the ViewModel, all the logic of the provider is stored in the provider class.

The logic behind the initialization of this class is very similar to the one behind the [Consumer](#the-consumer-model)  
in this class we register the provider through the Application runtime.

```java
private HelloWorldProvider provider;

private HelloWorldProviderApplication app;

public void setApp(final HelloWorldProviderApplication app) {
	this.app = app;
}

public void init() {
	...
	final Future<Void> future = app.getRuntime().registerProvider("domain", provider, providerQos);
	...
}
```

And after that the Model just waits for the the data in the provider to be updated and passes that  
information to the ViewModel.

##### The Provider class

This class is very similar to the [Java](./java.md#building-a-java-provider-application) class with the 
adition of livedata in order watch data  
changes and update the data outside the provider. An example of how to use this live data is  
provided here:

```java
public MutableLiveData<List<String>> m_requests;
```

This variable holds a register of all the requests made in order to display them in the  
application user interface.

### The ModelView Class

This is the component that will interact with the model and can be observed by the view. It only  
works as a bridge bettween the View and the Model without making any one of those dependant on  
eachother.

The ModelView holds the Model instance that holds the data that is used to update the user  
interface. In this class the model is initialized and iteracted with

```java
private final joynrModel model = new HelloWorldConsumerModel();

public void init(final HelloWorldConsumerApplication app) {
	model.setApp(app);
	model.init();
}
```

Then there is the need of the View knowing when the data has changed and updating itself. For this  
the follwoing method is created and the View observes it.

```java
	public LiveData<String> onUpdatedString() {
		return model.text;
	}
```

This method is triggered when the data in the Model is altered and it triggers an event on View.

You can create methods in the ModelView that interact with Model and can be called by the View,  
enabling a separation.

```java
public void onClick() {model.onClick();}
```

### The View class

The View class is where the presentation happens normally represented by the activities and  
fragments and is used to present information to the user. For this it needs to hold an instance  
of the ModelView so that it can fetch the data from model.

The ViewModel is initialized and binded to the View on the View creation.

```java
mcvm = ViewModelProviders.of(this).get(HelloWorldConsumerViewModel.class);
```

After creating and binding the ViewModel the *onUpdatedString* is set to be observed and the  
behaviour is set in the form of a method.

```java
public void bindData() {
	mcvm.onUpdatedString().observe(this, this::updateText);
}

private void updateText(final String text) {
	final TextView view = findViewById(R.id.text_box);
	view.setText(text);
}
```