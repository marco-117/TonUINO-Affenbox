
          agent any
        
        
          
  stages {
        
        
          
    stage('Build') {
        
        
          
      steps {
        
        
          
        sh '''pio account logout || true
        
        
          
PLATFORMIO_AUTH_TOKEN=${BUILD_TOKEN} pio remote run -r
        
        
          
'''
        
        
          
      }
        
        
          
    }
        
        
          


        
        
          
    stage('Software test') {
        
        
          
      steps {
        
        
          
        sh '''pio account logout || true
        
        
          
PLATFORMIO_AUTH_TOKEN=${TEST_TOKEN} pio remote test -e native -r'''
        
        
          
        sh '''pio account logout || true
        
        
          
PLATFORMIO_AUTH_TOKEN=${TEST_TOKEN} pio remote test -r'''
        
        
          
      }
        
        
          
    }
        
        
          


        
        
          
    stage('Hardware test') {
        
        
          
      agent {
        
        
          
        label 'PlatformIO-slave'
        
        
          
      }
        
        
          
      steps {
        
        
          
        sh '''/home/jenkins/.local/bin/pio run -e uno -t upload --upload-port /dev/ttyUSB0
        
        
          
sleep 5
        
        
          
python test_scripts/check.py'''
        
        
          
      }
        
        
          
    }
        
        
          


        
        
          
    stage('Deploy') {
        
        
          
      agent {
        
        
          
        label 'PlatformIO-slave'
        
        
          
      }
        
        
          
      when {
        
        
          
            branch "master"
        
        
          
        }
        
        
          
      steps {
        
        
          
        sh '''/home/jenkins/.local/bin/pio run -e megaatmega2560 -t upload --upload-port /dev/ttyUSB1
        
        
          
'''
        
        
          
      }
        
        
          
    }
        
        
          


        
        
          
  }
        
        
          
  environment {
        
        
          
    BUILD_TOKEN = credentials('BUILD_TOKEN')
        
        
          
    TEST_TOKEN = credentials('TEST_TOKEN')
        
        
          
  }
        
        
          
}pipeline {
        
          
