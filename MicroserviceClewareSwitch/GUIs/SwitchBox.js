// Function to execute when content inside the div changes
function handleSwitchBoxContent() {
   console.log('Content inside the div has changed');
   const result = global.clewareState;
   const deviceNumberSelect = document.getElementById('deviceNumber');
   const selectedValue = deviceNumberSelect.value;
   const valuesArray = Object.values(result[selectedValue]);
   updateIndicators(valuesArray);
   // Add your logic here for handling the content change
 }
 
 const mapValue = {
   'in1' : 0,
   'in2' : 1,
   'out1' : 0,
   'out2' : 1,
   'out3' : 2,
   'out4' : 3,
 }
 const exampleArray = [1, 0, 0, 0, 0, 0, 0, 0];

 function toggleLED(element) {
   element.classList.toggle('led-on');
 }

 function toggleLED(element, value) {
  //  const containerId = element.parentElement.id;

 
  //  // Get all LED indicators within the same container
  //  const allLEDs = document.querySelectorAll(`#${containerId} .led`);
 
  //  // Toggle the LED classes
  //  allLEDs.forEach(led => {
  //    if (led === element) {
  //      led.classList.toggle('led-on');
  //    } else {
  //     led.classList.remove('led-on');
  //   }
  //  });
  element.classList.toggle('led-on');
  const currentState = element.classList.contains('led-on') ? 'on' : 'off';
  console.log(`Current state: ${currentState}`);
  //  const otherLEDValue = getOtherLEDValue(containerId === 'usbIn' ? 'usbOut' : 'usbIn');
   updateSwitch(value, currentState);
 }
 
//  function toggleUSBIn(element, value) {
//    if (!element.classList.contains('led-on')) {
//      const usbInIndicators = document.querySelectorAll('.led-container .led-indicator');
//      usbInIndicators.forEach(indicator => indicator.classList.remove('led-on'));
//      element.classList.add('led-on');
//      const otherLEDValue = getOtherLEDValue('out');
//      updateSwitch(value, otherLEDValue);
//    } 
//  }
 
//  function toggleUSBOut(element, value) {
//    if (!element.classList.contains('led-on')) {
//      const usbOutIndicators = document.querySelectorAll('.led-container .led-indicator');
//      usbOutIndicators.forEach(indicator => indicator.classList.remove('led-on'));
//      element.classList.add('led-on');
//      const otherLEDValue = getOtherLEDValue('in');
//      updateSwitch(value, otherLEDValue);
//    }
//  }
 
 function getOtherLEDValue(containerId) {
   const otherLEDs = document.querySelectorAll(`#${containerId} .led-indicator.led-on`);
   const otherLED = Array.from(otherLEDs).find(led => led.classList.contains('led-on'));
   return mapValue[otherLED.id]; // Assuming LED values are stored in the 'data-value' attribute
 }
 
 function updateSwitch(ledIndex, onOff) {
   console.log(`LED Value: ${ledIndex}, `);
   var index = ledIndex;

   const hexString = "0x10";
   const sw0_idx = parseInt(hexString, 16);

   const deviceNumberSelect = document.getElementById('deviceNumber');
   const selectedValue = deviceNumberSelect.value;
   // Perform actions based on LED values
   var jsonData = {
      'method': 'svc_api_set_switch',
      'args': [selectedValue, sw0_idx + index, onOff]
    };
    requestClewareService(jsonData)
      .then(function (data) {
         console.log(`Set sw return: ${data}`);
      })
      .catch(function (error) {
        console.error('Error loading data:', error);
      });
 }

 function updateIndicators(array) {
   const allIndicators = document.querySelectorAll('.led');

   allIndicators.forEach((indicator, i) => {
    if (array[i] === 1)
      indicator.classList.toggle('led-on', true);
    else
     indicator.classList.toggle('led-on', false);
   });

  //  const onIndex = array.indexOf(1);
  //  if (onIndex === -1)
  //  {
  //     const inElement = document.getElementById('led1');
  //     inElement.classList.toggle('led-on');
  //     return;
  //  }

  //  const ledElement =  document.getElementById('led' + (onIndex + 1 ));
  //  ledElement.classList.add('led-on', true);
}

var amqp = require('amqplib/callback_api');

amqp.connect('amqp://localhost', function(error0, connection) {
  if (error0) {
    throw error0;
  }
  connection.createChannel(function(error1, channel) {
    if (error1) {
      throw error1;
    }
    var exchange = 'updates_sw_state';

    channel.assertExchange(exchange, 'fanout', {
      durable: false
    });

    channel.assertQueue('', {
      exclusive: true
    }, function(error2, q) {
      if (error2) {
        throw error2;
      }
      console.log(" [*] Waiting for messages in %s. To exit press CTRL+C", q.queue);
      channel.bindQueue(q.queue, exchange, '');

      channel.consume(q.queue, function(msg) {
        if(msg.content) {
            console.log(" [x] %s", msg.content.toString());
            const result = JSON.parse(msg.content.toString());
            const deviceNumberSelect = document.getElementById('deviceNumber');
            const selectedValue = deviceNumberSelect.value;
            const valuesArray = Object.values(result[selectedValue]);
            updateIndicators(valuesArray);
          }
      }, {
        noAck: true
      });
    });
  });
});

// receiveUpdates().catch(console.error);
 
 // Select the target div to observe
//  const targetDiv = document.getElementById('SpecificClewareDevice'); // Replace 'yourDivId' with your div's ID
 
//  // Create a new MutationObserver
//  const observer = new MutationObserver(function(mutations) {
//    mutations.forEach(function(mutation) {
//      if (mutation.type === 'childList') {
//        // Call your function when the content inside the div changes
//        handleMultiplexerContent();
//      }
//    });
//  });
 
//  // Configuration of the MutationObserver
//  const config = {
//    attributes: true,
//    childList: true,
//    subtree: true
//  };
 
//  // Start observing the target div for changes
//  observer.observe(targetDiv, config);
 
 // To disconnect the observer when needed
 // observer.disconnect();
 