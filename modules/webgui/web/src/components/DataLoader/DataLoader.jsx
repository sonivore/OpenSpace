import React, { Component } from 'react';
import { Link } from 'react-router-dom';
import { connect } from 'react-redux';
import Proptypes from 'prop-types'; 

import DataManager from '../../api/DataManager';
import styles from './DataLoader.scss';
import Window from '../common/Window/Window';
import { setActivated, setFilePaths } from '../../api/Actions/dataLoaderActions';
import Button from '../common/Input/Button/Button';
import Label from '../common/Label/Label';

class DataLoader extends Component {
  constructor(props) {
    super(props);

    this.dataTypesToLoad = ['Volumes', 'Fieldlines'];

    this.state = {
      activeDataType: '',
      dataToLoadUri: '',
    };
  }

  // handleChange(event) {
  //   let filePathString = event.target.value;
  // };

  shouldComponentUpdate(nextProps, nextState) {
    const { activeDataType, dataToLoadUri } = this.state;
    console.log(this.state)
    console.log(nextState)
    if ((activeDataType !== nextState.activeDataType) && (nextState.activeDataType !== '')) {
      this.triggerDataToLoad(nextState.activeDataType);
      this.setState({
        dataToLoadUri: this.getUriForDataToLoad(nextState.activeDataType)
      });
    }

    if (dataToLoadUri !== nextState.dataToLoadUri) {
      this.subscribeToActiveUri();
    }

    return true;
  }

  getUriForDataToLoad(dataType) {
    let uri = 'Modules.DataLoader.Reader.';

    for (const type of this.dataTypesToLoad) {
      uri += type;
    }

    console.log(`returning uri ${uri}`)

    return uri;
  }

  triggerDataToLoad(dataType) {
    DataManager.trigger(`Modules.DataLoader.Reader.Read${dataType}Trigger`)
  }

  handleDataTypeList(data) {
    console.log(data);
  }

  subscribeToActiveUri() {
    console.log(`subscribing to ${this.state.dataToLoadUri}`);
    DataManager.subscribe(this.state.dataToLoadUri, this.handleDataTypeList);
  }

  render() {
    const {setActivated, activated } = this.props
    console.log(this.state.dataToLoadUri)

    let dataTypeButtons = () => {
      return(
        <section className={styles.dataButtons}>
          <Label>
            Select data type you wish to load
          </Label>
          <div>
            {this.dataTypesToLoad.map((dataType) => 
              <Button 
                key={dataType} 
                onClick={() => this.setState({activeDataType: dataType})}>
                <Label>{dataType}</Label>
              </Button>
            )}
          </div>
        </section>
      );
    };

    let uploadDataButton = () => {
      return(
        <div>
          <label>
            <input 
              type="file"
              style={{opacity: 0}} 
              // onChange={(event) => this.handleChange(event)}
              // accept=".cdf, .osfls"/>
              multiple
              />    
            Upload Data
            {/* Style above - Or replace with <Button/> */}
          </label>
        </div>
      );
    };

    return(
      <div id="page-content-wrapper">
        { this.props.activated && (
          <div className={styles.center-content}>
            <Window
              title="Data Loader"
              // Temporary position and size fix
              size={{ width:600, height:400 }}
              position={{ x:470, y:-370 }}
              closeCallback={() => setActivated(false)}
            >
              { dataTypeButtons() }
              { uploadDataButton() }
            </Window>
          </div>
        )}
      </div>
    );
  }
}

const mapStateToProps = state => ({
  activated: state.dataLoader.activated
});

const mapDispatchToProps = dispatch => ({
  setFilePaths: (filePaths) => {
    dispatch(setFilePaths(filePaths))
  },

  setActivated: (isActivated) => {
    dispatch(setActivated(isActivated));
  },
});

DataLoader = connect(
  mapStateToProps,
  mapDispatchToProps
)(DataLoader);

export default DataLoader;