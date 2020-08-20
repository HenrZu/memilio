import * as am4core from '@amcharts/amcharts4/core';
import * as am4charts from '@amcharts/amcharts4/charts';
import * as am4maps from '@amcharts/amcharts4/maps';

export default class InteractiveHeatMap {
  /** @private
   *  @type MapChart */
  _map = null;

  /** @private
   *  @type MapPolygonSeries */
  _stateChoroplethSeries = null;

  /** @private
   *  @type MapPolygonSeries */
  _stateBackgroundSeries = null;

  /** @private
   *  @type Map<number, MapPolygonSeries> */
  _countySeries = new Map();

  /** @private
   * @type HeatLegend */
  _stateHeatLegend = null;

  /** @private
   *  @type Map<number, HeatLegend> */
  _countyHeatLegends = new Map();

  /** @private
   *  @type am4core.Label */
  _dataSetLabel = null;

  /** @private
   *  @type IHeatRule */
  _stateHeatRule = null;

  /** @private
   *  @type boolean */
  _seriesHit = false;

  /** @type number */
  selectedState = -1;

  /** @type number */
  selectedCounty = -1;

  /** @callback StateSelectedCallback
   *  @param newState {StateMetaData} */
  onStateSelected = (newState) => {};

  /** @callback CountySelectedCallback
   *  @param newCounty {any} */
  onCountySelected = (newCounty) => {};

  constructor(id) {
    this._map = am4core.create(id, am4maps.MapChart);
    this._map.projection = new am4maps.projections.Mercator();
    this._map.hiddenState.transitionDuration = 100;
    this._map.maxPanOut = 0.0;

    this._dataSetLabel = this._map.createChild(am4core.Label);
    this._dataSetLabel.align = 'right';
    this._dataSetLabel.valign = 'bottom';
    this._dataSetLabel.marginBottom = 20;
    this._dataSetLabel.marginRight = 20;
    this._dataSetLabel.text = 'Dataset: RKI';

    this._stateBackgroundSeries = new am4maps.MapPolygonSeries();
    this._stateBackgroundSeries.geodataSource.url = 'assets/german-states.geojson';
    this._stateBackgroundSeries.useGeodata = true;
    this._map.series.push(this._stateBackgroundSeries);
    this._stateBackgroundSeries.mapPolygons.template.fill = am4core.color('#AAA');
    this._stateBackgroundSeries.mapPolygons.template.tooltipText = '{name}';
    this._stateBackgroundSeries.hide();

    this._stateChoroplethSeries = new am4maps.MapPolygonSeries();
    this._stateChoroplethSeries.geodataSource.url = 'assets/german-states.geojson';
    this._stateChoroplethSeries.useGeodata = true;
    this._map.series.push(this._stateChoroplethSeries);

    this._stateHeatRule = {
      property: 'fill',
      target: this._stateChoroplethSeries.mapPolygons.template,
      min: am4core.color('#DDD', 1),
      max: am4core.color('#F00', 1),
      logarithmic: true,
    };
    this._stateChoroplethSeries.heatRules.push(this._stateHeatRule);

    this._stateHeatLegend = this._map.createChild(am4charts.HeatLegend);
    this._stateHeatLegend.series = this._stateChoroplethSeries;
    this._stateHeatLegend.height = am4core.percent(90);
    this._stateHeatLegend.align = 'left';
    this._stateHeatLegend.valign = 'middle';
    this._stateHeatLegend.orientation = 'vertical';
    this._stateHeatLegend.marginLeft = 20;

    const statePolygonTemplate = this._stateChoroplethSeries.mapPolygons.template;
    statePolygonTemplate.tooltipText = '{name}: {value}';
    statePolygonTemplate.nonScalingStroke = true;
    statePolygonTemplate.applyOnClones = true;

    const selectStateEvent = (e) => {
      this._seriesHit = true;
      const item = e.target.dataItem.dataContext;
      this.onStateSelected(item);
      this.onCountySelected(null);
      this.selectedCounty = -1;
      this._stateSelected(item);
    };

    statePolygonTemplate.events.on('hit', selectStateEvent);
    this._stateBackgroundSeries.mapPolygons.template.events.on('hit', selectStateEvent);

    const zoomOutEvent = () => {
      this.onStateSelected(null);
      this.onCountySelected(null);
      this.selectedCounty = -1;
      this._stateSelected({id: -1});
    };

    this._map.events.on('hit', () => {
      if (!this._seriesHit) {
        zoomOutEvent();
      }
      this._seriesHit = false;
    });

    this._map.events.on('zoomlevelchanged', () => {
      if (this._map.zoomLevel === 1) {
        zoomOutEvent();
      }
    });

    for (let state of STATE_METADATA) {
      this._createCountySeries(state);
    }

    this._map.invalidate();
  }

  setDataSetName(name) {
    this._dataSetLabel.text = 'Dataset: ' + name;
  }

  /** @param values {Map<number, number>} */
  setStateValues(values) {
    if (values) {
      for (let stateDatum of this._stateChoroplethSeries.data) {
        stateDatum.value = values.get(stateDatum.id);
      }

      this._stateChoroplethSeries.invalidateRawData();
    }
  }

  /** @param values {Map<string, number>} */
  setCountyValues(values) {
    if (!values) {
      return;
    }

    if (this._countySeries.has(this.selectedState)) {
      const series = this._countySeries.get(this.selectedState);
      for (let countyDatum of series.data) {
        countyDatum.value = values.get(parseInt(countyDatum.RS, 10));
      }

      series.invalidateRawData();
    }
  }

  /** @private
   *  @param state {StateMetaData} */
  _createCountySeries(state) {
    const newSeries = new am4maps.MapPolygonSeries();
    newSeries.geodataSource.url = 'assets/counties/' + state.fileName + '.geojson';
    newSeries.useGeodata = true;
    newSeries.hidden = true;

    const countyPolygonTemplate = newSeries.mapPolygons.template;
    countyPolygonTemplate.tooltipText = '{id}: {value}';
    countyPolygonTemplate.nonScalingStroke = true;
    countyPolygonTemplate.applyOnClones = true;

    const countyHeatRule = {
      property: 'fill',
      target: countyPolygonTemplate,
      min: am4core.color('#DDD', 1),
      max: am4core.color('#F00', 1),
      logarithmic: true,
    };

    newSeries.heatRules.push(countyHeatRule);

    const heatLegend = this._map.createChild(am4charts.HeatLegend);
    heatLegend.series = newSeries;
    heatLegend.height = am4core.percent(90);
    heatLegend.align = 'left';
    heatLegend.valign = 'middle';
    heatLegend.orientation = 'vertical';
    heatLegend.marginLeft = 20;

    countyPolygonTemplate.events.on('hit', (e) => {
      this._seriesHit = true;
      const item = e.target.dataItem.dataContext;
      this.selectedCounty = item !== null ? parseInt(item.RS, 10) : -1;
      this.onCountySelected(item);
    });

    this._map.series.push(newSeries);
    this._countySeries.set(state.id, newSeries);
    this._countyHeatLegends.set(state.id, heatLegend);
  }

  /** @private
   *  @param newState {StateMetaData} */
  _stateSelected(newState) {
    // If a new state is selected hide the old one.
    if (this.selectedState !== newState.id && this._countySeries.has(this.selectedState)) {
      this._countySeries.get(this.selectedState).hide();
      this._countyHeatLegends.get(this.selectedState).hide();
    }

    if (newState.id > 0) {
      // A state is selected.
      // County series will be loaded lazily.
      if (this._countySeries.has(newState.id)) {
        this._countySeries.get(newState.id).show();
        this._countyHeatLegends.get(newState.id).show();
      } else {
        const state = stateById(newState.id);
        this._createCountySeries(state);
      }

      this.selectedState = newState.id;
      this._map.zoomToMapObject(this._stateChoroplethSeries.getPolygonById(newState.id));
      this._stateHeatLegend.hide();
      this._stateChoroplethSeries.hide();
      this._stateBackgroundSeries.show();
    } else {
      // No state is selected.
      this.selectedState = -1;
      this._map.goHome();
      this._stateHeatLegend.show();
      this._stateChoroplethSeries.show();
      this._stateBackgroundSeries.hide();
    }
  }
}

export class StateMetaData {
  /** @type number */
  id;

  /** @type string | undefined */
  name;

  /** @type string | undefined */
  fileName;
}

/** @type {Array<StateMetaData>} */
const STATE_METADATA = [
  {id: 1, name: 'Schleswig-Holstein', fileName: 'schleswig-holstein'},
  {id: 2, name: 'Hamburg', fileName: 'hamburg'},
  {id: 3, name: 'Niedersachsen', fileName: 'niedersachsen'},
  {id: 4, name: 'Bremen', fileName: 'bremen'},
  {id: 5, name: 'Nordrhein-Westfalen', fileName: 'nordrhein-westfalen'},
  {id: 6, name: 'Hessen', fileName: 'hessen'},
  {id: 7, name: 'Rheinland-Pfalz', fileName: 'rheinland-pfalz'},
  {id: 8, name: 'Baden-Württemberg', fileName: 'baden-wuerttemberg'},
  {id: 9, name: 'Bayern', fileName: 'bayern'},
  {id: 10, name: 'Saarland', fileName: 'saarland'},
  {id: 11, name: 'Berlin', fileName: 'berlin'},
  {id: 12, name: 'Brandenburg', fileName: 'brandenburg'},
  {id: 13, name: 'Mecklenburg-Vorpommern', fileName: 'mecklenburg-vorpommern'},
  {id: 14, name: 'Sachsen', fileName: 'sachsen'},
  {id: 15, name: 'Sachsen-Anhalt', fileName: 'sachsen-anhalt'},
  {id: 16, name: 'Thüringen', fileName: 'thueringen'},
];

/**
 * @param id {number}
 * @return {StateMetaData}
 */
function stateById(id) {
  return STATE_METADATA[id - 1];
}
