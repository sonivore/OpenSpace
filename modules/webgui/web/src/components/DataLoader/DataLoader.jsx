import React, { Component } from 'react';
import { Link } from 'react-router-dom';
import { connect } from 'react-redux';
import Proptypes from 'prop-types'; 

import DataItemList from './presentational/DataItemList';
import { stringListToArray } from './utils/helpers';

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

    this.handleDataTypeList = this.handleDataTypeList.bind(this);
    this.handleFilepaths = this.handleFilepaths.bind(this);


    this.state = {
      activeDataType: '',
      dataToLoadUri: '',
      filePath: ''
    };
  }


  handleClick() {
    this.subscribeToFilepaths();
    this.triggerFilesToLoad();
  }

  handleFilepaths(data) {
    this.setState({filePath:data.value});
  }

  shouldComponentUpdate(nextProps, nextState) {
    const { activeDataType, dataToLoadUri } = this.state;
    if ((activeDataType !== nextState.activeDataType) && (nextState.activeDataType !== '')) {
      this.triggerDataToLoad(nextState.activeDataType);
      const uri = this.getUriForDataToLoad(nextState.activeDataType);
      this.setState({ dataToLoadUri: uri }, this.subscribeToActiveUri(uri));
    }

    if (dataToLoadUri !== nextState.dataToLoadUri) {
      this.subscribeToActiveUri(nextState.dataToLoadUri);
    }

    return true;
  }

  getUriForDataToLoad(dataType) {
    let uri = 'Modules.DataLoader.Reader.';

    for (const type of this.dataTypesToLoad) {
      if (dataType == type) {
        uri += type;
      }
    }

    return uri;
  }

  triggerDataToLoad(dataType) {
    DataManager.trigger(`Modules.DataLoader.Reader.Read${dataType}Trigger`)
  }

  triggerFilesToLoad() {
    DataManager.trigger(`Modules.DataLoader.Reader.LoadDataTrigger`)
  }

  handleDataTypeList(data) {
    this.setState({dataItems: stringListToArray(data.Value)});
  }

  subscribeToActiveUri(uri = '') {
    DataManager.subscribe(uri || this.state.dataToLoadUri, this.handleDataTypeList);
  }

  subscribeToFilepaths() {
    DataManager.subscribe('Modules.DataLoader.Reader.LoadDataTrigger', this.handleFilepaths);
  }

  render() {
    const {setActivated, activated } = this.props

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
                onClick={() => this.setState({activeDataType: dataType})}
                disabled={dataType == 'Fieldlines'}>
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
          <Button onClick={() => this.handleClick()}>
            Load Data
          </Button>
        </div>
      );
    };

    return(
      <div className="page-content-wrapper">
        { this.props.activated && (
          <div className={styles.centerContent}>
            <Window
              title="Data Loader"
              // Temporary position and size fix
              size={{ width:600, height:400 }}
              position={{ x:470, y:-370 }}
              closeCallback={() => setActivated(false)}
            >
              { dataTypeButtons() }
              <DataItemList items={this.state.dataItems} />
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